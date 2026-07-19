#include "game/enhance.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

#include "game/config.h"
#include "game/gconfig.h"
#include "game/light.h"
#include "game/map.h"
#include "game/scripts.h"
#include "game/tile.h"
#include "game/weather.h"
#include "plib/color/color.h"
#include "plib/gnw/input.h"
#include "plib/gnw/memory.h"

namespace fallout {

#define FLICKER_BUCKET_COUNT 64

// Max extra darkening applied by the vignette at the far corners, in
// intensity table steps (128 = neutral).
#define VIGNETTE_MAX_DARKEN 26

static void enhance_bk();
static bool enhance_game_active();
static void enhance_update_tint();
static void enhance_update_flicker(unsigned int now);
static void enhance_time_of_day_tint(int* r, int* g, int* b);

static bool enhance_initialized = false;
static bool tod_tint_enabled = true;
static bool flicker_enabled = true;
static bool shadows_enabled = true;
static bool vignette_enabled = true;

static unsigned int last_bk_time;
static unsigned int last_refresh_time;

// Current flicker offsets, roughly -256..256, one per phase bucket.
static int flicker_wave[FLICKER_BUCKET_COUNT];

// Screen-space horizontal shear of critter shadows (-128..128, applied as
// x += shear * row / 256). Follows the sun during the day.
static int shadow_shear = 64;

static unsigned char* vignette_map = NULL;
static int vignette_width = 0;
static int vignette_height = 0;

int enhance_init()
{
    int value;

    tod_tint_enabled = true;
    if (config_get_value(&game_config, "enhancements", "time_of_day_tint", &value)) {
        tod_tint_enabled = value != 0;
    }

    flicker_enabled = true;
    if (config_get_value(&game_config, "enhancements", "light_flicker", &value)) {
        flicker_enabled = value != 0;
    }

    shadows_enabled = true;
    if (config_get_value(&game_config, "enhancements", "shadows", &value)) {
        shadows_enabled = value != 0;
    }

    vignette_enabled = true;
    if (config_get_value(&game_config, "enhancements", "vignette", &value)) {
        vignette_enabled = value != 0;
    }

    weather_init();

    last_bk_time = get_time();
    last_refresh_time = last_bk_time;
    memset(flicker_wave, 0, sizeof(flicker_wave));

    add_bk_process(enhance_bk);

    enhance_initialized = true;

    return 0;
}

void enhance_reset()
{
    weather_reset();
    enhance_update_tint();
}

void enhance_exit()
{
    if (enhance_initialized) {
        remove_bk_process(enhance_bk);
        colorSetDisplayTint(256, 256, 256);
        weather_exit();

        if (vignette_map != NULL) {
            mem_free(vignette_map);
            vignette_map = NULL;
            vignette_width = 0;
            vignette_height = 0;
        }

        enhance_initialized = false;
    }
}

bool enhance_shadows_enabled()
{
    return shadows_enabled;
}

int enhance_render_light(int elevation, int tile)
{
    int ambient = light_get_ambient();
    int intensity = light_get_tile(elevation, tile);

    if (flicker_enabled && intensity > ambient) {
        int excess = intensity - ambient;
        int wave = flicker_wave[(unsigned int)(tile * 2654435761u) >> 26];
        excess += ((excess * wave) >> 8) * 20 >> 8;
        intensity = ambient + excess;
    }

    if (intensity < ambient) {
        intensity = ambient;
    }

    int dim = weather_light_dim();
    if (dim != 256) {
        intensity = intensity * dim >> 8;
        if (intensity < LIGHT_LEVEL_MIN) {
            intensity = LIGHT_LEVEL_MIN;
        }
    }

    if (intensity > LIGHT_LEVEL_MAX) {
        intensity = LIGHT_LEVEL_MAX;
    }

    return intensity;
}

int enhance_render_ambient()
{
    int ambient = light_get_ambient();

    int dim = weather_light_dim();
    if (dim != 256) {
        ambient = ambient * dim >> 8;
        if (ambient < LIGHT_LEVEL_MIN) {
            ambient = LIGHT_LEVEL_MIN;
        }
    }

    return ambient;
}

// Projects the silhouette flattened 4:1 onto the ground at the critter's
// feet, sheared horizontally by the current sun direction. Destination
// pixels are darkened through the intensity table, so the result stays in
// the original palette. Special colors (0xE5+, used for glows and color
// cycling) are left untouched.
void enhance_render_shadow(unsigned char* frameData, int frameWidth, int frameHeight,
    int sx, int sy, Rect* clipRect, unsigned char* dest, int destPitch, int light)
{
    if (frameHeight < 8) {
        return;
    }

    int feetY = sy + frameHeight - 1;
    int shadowHeight = (frameHeight - 1) / 4;

    // Shadows read stronger in brightly lit areas.
    int darkIndex = 128 - (10 + 30 * light / LIGHT_LEVEL_MAX);

    for (int row = 0; row <= shadowHeight; row++) {
        int y = feetY - row;
        if (y < clipRect->uly || y > clipRect->lry) {
            continue;
        }

        int srcRow = frameHeight - 1 - row * 4;
        if (srcRow < 0) {
            break;
        }

        unsigned char* srcLine = frameData + frameWidth * srcRow;
        unsigned char* destLine = dest + destPitch * y;
        int xShift = sx + (shadow_shear * row >> 8);

        for (int x = 0; x < frameWidth; x++) {
            if (srcLine[x] == 0) {
                continue;
            }

            int dx = xShift + x;
            if (dx < clipRect->ulx || dx > clipRect->lrx) {
                continue;
            }

            unsigned char c = destLine[dx];
            if (c < 0xE5) {
                destLine[dx] = intensityColorTable[c][darkIndex];
            }
        }
    }
}

void enhance_scene_post_process(unsigned char* buf, int pitch, Rect* rect, int bufWidth, int bufHeight)
{
    if (!enhance_initialized) {
        return;
    }

    weather_render(buf, pitch, rect, bufWidth, bufHeight);

    if (vignette_enabled) {
        if (vignette_map == NULL || vignette_width != bufWidth || vignette_height != bufHeight) {
            if (vignette_map != NULL) {
                mem_free(vignette_map);
            }

            vignette_map = (unsigned char*)mem_malloc(bufWidth * bufHeight);
            if (vignette_map == NULL) {
                return;
            }

            vignette_width = bufWidth;
            vignette_height = bufHeight;

            double halfWidth = bufWidth / 2.0;
            double halfHeight = bufHeight / 2.0;
            for (int y = 0; y < bufHeight; y++) {
                double ny = (y - halfHeight) / halfHeight * 1.12;
                for (int x = 0; x < bufWidth; x++) {
                    double nx = (x - halfWidth) / halfWidth;
                    double r = sqrt(nx * nx + ny * ny);
                    double t = (r - 0.62) / 0.55;
                    if (t < 0.0) {
                        t = 0.0;
                    }
                    if (t > 1.0) {
                        t = 1.0;
                    }
                    vignette_map[bufWidth * y + x] = (unsigned char)(t * t * VIGNETTE_MAX_DARKEN);
                }
            }
        }

        for (int y = rect->uly; y <= rect->lry; y++) {
            unsigned char* p = buf + pitch * y + rect->ulx;
            unsigned char* v = vignette_map + vignette_width * y + rect->ulx;
            for (int x = rect->ulx; x <= rect->lrx; x++) {
                if (*v != 0 && *p < 0xE5) {
                    *p = intensityColorTable[*p][128 - *v];
                }
                p++;
                v++;
            }
        }
    }
}

// Mood systems only act while a map is loaded - the main menu, credits and
// movies keep a neutral palette.
static bool enhance_game_active()
{
    return map_data.name[0] != '\0';
}

static void enhance_bk()
{
    unsigned int now = get_time();
    if (elapsed_tocks(now, last_bk_time) < 33) {
        return;
    }
    last_bk_time = now;

    if (!enhance_game_active()) {
        colorSetDisplayTint(256, 256, 256);
        return;
    }

    weather_update();
    enhance_update_flicker(now);
    enhance_update_tint();

    // Figure out how often the scene needs unprompted redraws: weather
    // particles want ~30 fps, flicker is happy with ~10. Skip flicker
    // refreshes when ambient light is nearly maxed - above-ambient light
    // sources can't stand out and the flicker would be invisible.
    unsigned int interval = 0;
    if (weather_has_visuals()) {
        interval = 33;
    } else if (flicker_enabled && light_get_ambient() < LIGHT_LEVEL_MAX * 9 / 10) {
        interval = 100;
    }

    if (interval != 0 && elapsed_tocks(now, last_refresh_time) >= interval) {
        last_refresh_time = now;
        tile_refresh_display();
    }
}

static void enhance_update_flicker(unsigned int now)
{
    if (!flicker_enabled) {
        return;
    }

    double t = now / 1000.0;
    for (int index = 0; index < FLICKER_BUCKET_COUNT; index++) {
        double phase = index * 0.7;
        double wave = sin(t * 9.0 + phase) * 0.55 + sin(t * 15.3 + phase * 1.9) * 0.45;
        flicker_wave[index] = (int)(wave * 256.0);
    }

    // Sun direction for shadows: sweeps across the day, overhead at noon.
    int minutes = game_time() / 600 % 1440;
    if (minutes >= 6 * 60 && minutes <= 18 * 60) {
        shadow_shear = 120 - (minutes - 6 * 60) * 240 / (12 * 60);
    } else {
        shadow_shear = 0;
    }
}

static void enhance_update_tint()
{
    int todR = 256;
    int todG = 256;
    int todB = 256;
    if (tod_tint_enabled) {
        enhance_time_of_day_tint(&todR, &todG, &todB);
    }

    int weatherR;
    int weatherG;
    int weatherB;
    weather_get_tint(&weatherR, &weatherG, &weatherB);

    colorSetDisplayTint(todR * weatherR >> 8, todG * weatherG >> 8, todB * weatherB >> 8);
}

// Piecewise-linear mood curve over the game clock: cool nights, warm dawns
// and dusks, neutral during the day.
static void enhance_time_of_day_tint(int* r, int* g, int* b)
{
    static const int keys[][4] = {
        // minute, r, g, b
        { 0, 208, 214, 248 },
        { 270, 208, 214, 248 }, // 4:30 night
        { 390, 255, 228, 206 }, // 6:30 dawn
        { 510, 256, 256, 256 }, // 8:30 day
        { 990, 256, 256, 256 }, // 16:30 day
        { 1110, 255, 221, 198 }, // 18:30 dusk
        { 1230, 208, 214, 248 }, // 20:30 night
        { 1440, 208, 214, 248 },
    };

    int minutes = game_time() / 600 % 1440;

    for (int index = 0; index < (int)(sizeof(keys) / sizeof(keys[0])) - 1; index++) {
        if (minutes >= keys[index][0] && minutes <= keys[index + 1][0]) {
            int span = keys[index + 1][0] - keys[index][0];
            int pos = minutes - keys[index][0];
            *r = keys[index][1] + (keys[index + 1][1] - keys[index][1]) * pos / span;
            *g = keys[index][2] + (keys[index + 1][2] - keys[index][2]) * pos / span;
            *b = keys[index][3] + (keys[index + 1][3] - keys[index][3]) * pos / span;
            return;
        }
    }
}

} // namespace fallout
