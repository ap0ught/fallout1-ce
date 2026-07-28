#include "game/enhance.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

#include <algorithm>

#include "game/art.h"
#include "game/config.h"
#include "game/gconfig.h"
#include "game/light.h"
#include "game/map.h"
#include "game/object.h"
#include "game/scripts.h"
#include "game/tile.h"
#include "game/weather.h"
#include "plib/color/color.h"
#include "plib/gnw/input.h"
#include "plib/gnw/memory.h"

namespace fallout {

// Flicker is sampled on a coarse grid of cells (1 << FLICKER_CELL_SHIFT tiles
// per cell) and interpolated across tiles, so neighbouring tiles stay in step
// instead of each picking an unrelated phase. The grid wraps, which keeps it
// tiny; the repeat period is far wider than the screen.
#define FLICKER_CELL_SHIFT 3
#define FLICKER_GRID 8

// Tint buckets for colored light: 0-1 cool, 2 neutral, 3-4 warm.
#define TINT_BUCKET_COUNT 5
#define TINT_BUCKET_NEUTRAL 2

#define WARMTH_CACHE_SIZE 32

// Max extra darkening applied by the vignette at the far corners, in
// intensity table steps (128 = neutral).
#define VIGNETTE_MAX_DARKEN 26

static void enhance_bk();
static bool enhance_game_active();
static void enhance_update_tint();
static void enhance_update_flicker(unsigned int now);
static int enhance_flicker_at(const int (*wave)[FLICKER_GRID], int tile);
static int enhance_flicker_value(int elevation, int tile);
static void enhance_time_of_day_tint(int* r, int* g, int* b);
static int enhance_classify_warmth(Object* obj);
static void enhance_build_tint_tables();
static void enhance_rebuild_ao();

static bool enhance_initialized = false;

// A/B comparison state. `compare_mode` is one of ENHANCE_COMPARE_*;
// `enhance_bypass` is flipped on transiently by the renderer to force vanilla
// output for a pass (see enhance_set_bypass).
static int compare_mode = ENHANCE_COMPARE_NEW;
static bool enhance_bypass = false;

static bool tod_tint_enabled = true;
static bool flicker_enabled = true;
static bool shadows_enabled = true;
static bool vignette_enabled = true;
static bool colored_lights_enabled = true;
static bool wall_ao_enabled = true;

static unsigned int last_bk_time;
static unsigned int last_refresh_time;

// Current flicker offsets, roughly -256..256, one per grid cell. Flames need a
// restless wave, but lamps and daylight spilling in through a window only ever
// drift - so two waves are kept and blended per tile by how warm its light is.
static int flicker_fast[FLICKER_GRID][FLICKER_GRID];
static int flicker_slow[FLICKER_GRID][FLICKER_GRID];

// Screen-space horizontal shear of critter shadows (-128..128, applied as
// x += shear * row / 256). Follows the sun during the day.
static int shadow_shear = 64;

static unsigned char* vignette_map = NULL;
static int vignette_width = 0;
static int vignette_height = 0;

// Warmth of the light source currently being distributed by
// `obj_adjust_light` (-256 cool .. 256 warm, 0 neutral).
static int light_source_warmth = 0;

// Per-tile accumulated warmth-weighted source light (intensity * warmth >> 8,
// mirroring every light_add_to_tile/light_subtract_from_tile call).
static int warm_accum[ELEVATION_COUNT][HEX_GRID_SIZE];

static struct {
    int fid;
    int warmth;
} warmth_cache[WARMTH_CACHE_SIZE];
static int warmth_cache_count = 0;
static int warmth_cache_cursor = 0;

static unsigned char tinted_intensity_table[TINT_BUCKET_COUNT][256][256];
static bool tinted_tables_built = false;

// Wall-contact ambient occlusion, 0..255 fraction of light removed.
static unsigned char ao_map[ELEVATION_COUNT][HEX_GRID_SIZE];
static char ao_built_for[16];

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

    colored_lights_enabled = true;
    if (config_get_value(&game_config, "enhancements", "colored_lights", &value)) {
        colored_lights_enabled = value != 0;
    }

    wall_ao_enabled = true;
    if (config_get_value(&game_config, "enhancements", "wall_ao", &value)) {
        wall_ao_enabled = value != 0;
    }

    enhance_light_color_reset();
    memset(ao_map, 0, sizeof(ao_map));
    ao_built_for[0] = '\0';
    warmth_cache_count = 0;
    warmth_cache_cursor = 0;

    weather_init();

    last_bk_time = get_time();
    last_refresh_time = last_bk_time;
    memset(flicker_fast, 0, sizeof(flicker_fast));
    memset(flicker_slow, 0, sizeof(flicker_slow));

    add_bk_process(enhance_bk);

    enhance_initialized = true;

    return 0;
}

void enhance_reset()
{
    weather_reset();
    memset(ao_map, 0, sizeof(ao_map));
    ao_built_for[0] = '\0';
    enhance_update_tint();
}

void enhance_map_changed()
{
    memset(ao_map, 0, sizeof(ao_map));
    ao_built_for[0] = '\0';
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
    return shadows_enabled && !enhance_bypass;
}

int enhance_compare_mode()
{
    return compare_mode;
}

void enhance_set_bypass(bool bypass)
{
    enhance_bypass = bypass;
}

const char* enhance_compare_cycle()
{
    compare_mode = (compare_mode + 1) % ENHANCE_COMPARE_COUNT;

    // The palette tint is a display-wide effect that cannot be split spatially,
    // so update it up front for OLD/NEW and refresh the whole scene.
    enhance_update_tint();
    tile_refresh_display();

    switch (compare_mode) {
    case ENHANCE_COMPARE_OLD:
        return "Enhancements: OFF (vanilla)";
    case ENHANCE_COMPARE_SPLIT:
        return "Enhancements: SPLIT (vanilla | enhanced)";
    default:
        return "Enhancements: ON";
    }
}

int enhance_render_light(int elevation, int tile)
{
    int ambient = light_get_ambient();
    int intensity = light_get_tile(elevation, tile);

    if (flicker_enabled && !enhance_bypass && intensity > ambient) {
        int excess = intensity - ambient;
        int wave = enhance_flicker_value(elevation, tile);
        excess += ((excess * wave) >> 8) * 20 >> 8;
        intensity = ambient + excess;
    }

    if (intensity < ambient) {
        intensity = ambient;
    }

    if (!enhance_bypass) {
        int dim = weather_light_dim();
        if (dim != 256) {
            intensity = intensity * dim >> 8;
            if (intensity < LIGHT_LEVEL_MIN) {
                intensity = LIGHT_LEVEL_MIN;
            }
        }
    }

    if (wall_ao_enabled && !enhance_bypass && elevationIsValid(elevation) && hexGridTileIsValid(tile)) {
        int ao = ao_map[elevation][tile];
        if (ao != 0) {
            intensity -= intensity * ao >> 8;
            if (intensity < LIGHT_LEVEL_MIN) {
                intensity = LIGHT_LEVEL_MIN;
            }
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

    if (!enhance_bypass) {
        int dim = weather_light_dim();
        if (dim != 256) {
            ambient = ambient * dim >> 8;
            if (ambient < LIGHT_LEVEL_MIN) {
                ambient = LIGHT_LEVEL_MIN;
            }
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
    if (!enhance_initialized || enhance_bypass) {
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

void enhance_light_source_begin(Object* obj)
{
    // Warmth also drives flicker speed, so it is classified whenever either
    // feature is on.
    light_source_warmth = (colored_lights_enabled || flicker_enabled) ? enhance_classify_warmth(obj) : 0;
}

void enhance_light_source_end()
{
    light_source_warmth = 0;
}

void enhance_light_color_add(int elevation, int tile, int intensity)
{
    if (light_source_warmth != 0) {
        warm_accum[elevation][tile] += intensity * light_source_warmth >> 8;
    }
}

void enhance_light_color_subtract(int elevation, int tile, int intensity)
{
    if (light_source_warmth != 0) {
        warm_accum[elevation][tile] -= intensity * light_source_warmth >> 8;
    }
}

void enhance_light_color_reset()
{
    memset(warm_accum, 0, sizeof(warm_accum));
}

unsigned char (*enhance_light_table(int elevation, int tile))[256]
{
    if (!colored_lights_enabled || enhance_bypass || !elevationIsValid(elevation) || !hexGridTileIsValid(tile)) {
        return intensityColorTable;
    }

    // Source-contributed light on this tile (655 is the empty-tile base).
    int sourceLight = light_get_tile_true(elevation, tile) - 655;
    if (sourceLight < 1024) {
        return intensityColorTable;
    }

    int accum = warm_accum[elevation][tile];
    if (accum > -16 && accum < 16) {
        return intensityColorTable;
    }

    if (!tinted_tables_built) {
        enhance_build_tint_tables();
    }

    // Average warmth of the source light, then weighted by how much of the
    // visible light actually comes from sources rather than ambient.
    int warmth = accum * 256 / sourceLight;
    warmth = std::clamp(warmth, -256, 256);

    int ambient = light_get_ambient();
    int total = std::min(std::max(ambient, 655 + sourceLight), LIGHT_LEVEL_MAX);
    int excess = total - ambient;
    if (excess <= 0) {
        return intensityColorTable;
    }
    warmth = warmth * excess / total;

    int bucket;
    if (warmth >= 0) {
        bucket = TINT_BUCKET_NEUTRAL + (warmth + 48) / 96;
    } else {
        bucket = TINT_BUCKET_NEUTRAL - (48 - warmth) / 96;
    }
    bucket = std::clamp(bucket, 0, TINT_BUCKET_COUNT - 1);

    return tinted_intensity_table[bucket];
}

// Estimates whether a light source is warm (fires, braziers) or cool
// (monitors, force fields) from the average palette color of its art.
// Critters and items are considered neutral emitters.
static int enhance_classify_warmth(Object* obj)
{
    int type = FID_TYPE(obj->fid);
    if (type != OBJ_TYPE_SCENERY && type != OBJ_TYPE_MISC) {
        return 0;
    }

    for (int index = 0; index < warmth_cache_count; index++) {
        if (warmth_cache[index].fid == obj->fid) {
            return warmth_cache[index].warmth;
        }
    }

    int warmth = 0;

    CacheEntry* handle;
    Art* art = art_ptr_lock(obj->fid, &handle);
    if (art != NULL) {
        unsigned char* data = art_frame_data(art, 0, 0);
        if (data != NULL) {
            int size = art_frame_width(art, 0, 0) * art_frame_length(art, 0, 0);
            int sumR = 0;
            int sumG = 0;
            int sumB = 0;
            for (int offset = 0; offset < size; offset += 3) {
                unsigned char c = data[offset];
                if (c == 0) {
                    continue;
                }
                sumR += cmap[3 * c];
                sumG += cmap[3 * c + 1];
                sumB += cmap[3 * c + 2];
            }

            int total = sumR + sumG + sumB;
            if (total > 0) {
                warmth = std::clamp((sumR - sumB) * 340 / total, -256, 256);
                if (warmth > -40 && warmth < 40) {
                    warmth = 0;
                }
            }
        }
        art_ptr_unlock(handle);
    }

    if (warmth_cache_count < WARMTH_CACHE_SIZE) {
        warmth_cache[warmth_cache_count].fid = obj->fid;
        warmth_cache[warmth_cache_count].warmth = warmth;
        warmth_cache_count++;
    } else {
        warmth_cache[warmth_cache_cursor].fid = obj->fid;
        warmth_cache[warmth_cache_cursor].warmth = warmth;
        warmth_cache_cursor = (warmth_cache_cursor + 1) % WARMTH_CACHE_SIZE;
    }

    return warmth;
}

// Builds warm/cool variants of `intensityColorTable`. Index 0 (transparent)
// and 0xE5+ (color cycling and glow specials) keep the neutral behavior so
// palette animation is never frozen by a tint.
static void enhance_build_tint_tables()
{
    for (int bucket = 0; bucket < TINT_BUCKET_COUNT; bucket++) {
        int s = bucket - TINT_BUCKET_NEUTRAL;

        int scaleR;
        int scaleG;
        int scaleB;
        if (s >= 0) {
            scaleR = 256 + 20 * s;
            scaleG = 256 + 2 * s;
            scaleB = 256 - 26 * s;
        } else {
            scaleR = 256 + 22 * s;
            scaleG = 256 + 5 * s;
            scaleB = 256 - 18 * s;
        }

        for (int color = 0; color < 256; color++) {
            memcpy(tinted_intensity_table[bucket][color], intensityColorTable[color], 256);

            if (s == 0 || color == 0 || color >= 0xE5) {
                continue;
            }

            int rgb = Color2RGB(color);
            int r = (rgb & 0x7C00) >> 10;
            int g = (rgb & 0x3E0) >> 5;
            int b = rgb & 0x1F;

            for (int level = 0; level <= 128; level++) {
                int rr = std::min(r * level / 128 * scaleR >> 8, 31);
                int gg = std::min(g * level / 128 * scaleG >> 8, 31);
                int bb = std::min(b * level / 128 * scaleB >> 8, 31);
                tinted_intensity_table[bucket][color][level] = colorTable[(rr << 10) | (gg << 5) | bb];
            }
        }
    }

    tinted_tables_built = true;
}

// Marks wall tiles and their neighbors for soft base darkening. The Gouraud
// floor lighting interpolates these per-tile values into smooth gradients.
static void enhance_rebuild_ao()
{
    memset(ao_map, 0, sizeof(ao_map));

    Object* obj = obj_find_first();
    while (obj != NULL) {
        if (FID_TYPE(obj->fid) == OBJ_TYPE_WALL
            && elevationIsValid(obj->elevation)
            && hexGridTileIsValid(obj->tile)) {
            unsigned char* ao = ao_map[obj->elevation];

            int self = ao[obj->tile] + 34;
            ao[obj->tile] = self > 72 ? 72 : self;

            for (int rotation = 0; rotation < ROTATION_COUNT; rotation++) {
                int neighbor = tile_num_in_direction(obj->tile, rotation, 1);
                if (hexGridTileIsValid(neighbor)) {
                    int value = ao[neighbor] + 22;
                    ao[neighbor] = value > 56 ? 56 : value;
                }
            }
        }

        obj = obj_find_next();
    }

    strncpy(ao_built_for, map_data.name, sizeof(ao_built_for) - 1);
    ao_built_for[sizeof(ao_built_for) - 1] = '\0';
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

    if (wall_ao_enabled && strncmp(ao_built_for, map_data.name, sizeof(ao_built_for)) != 0) {
        enhance_rebuild_ao();
    }

    weather_update();
    enhance_update_flicker(now);
    enhance_update_tint();

    // Figure out how often the scene needs unprompted redraws: weather
    // particles want ~30 fps, flicker needs ~15 to stay smooth. Skip flicker
    // refreshes when ambient light is nearly maxed - above-ambient light
    // sources can't stand out and the flicker would be invisible.
    unsigned int interval = 0;
    if (weather_has_visuals()) {
        interval = 33;
    } else if (flicker_enabled && light_get_ambient() < LIGHT_LEVEL_MAX * 9 / 10) {
        interval = 66;
    }

    if (interval != 0 && elapsed_tocks(now, last_refresh_time) >= interval) {
        last_refresh_time = now;
        tile_refresh_display();
    }
}

// Smoothstep on an 8-bit fraction, so cell seams have no visible crease.
static inline int enhance_smooth_frac(int f)
{
    return f * f * (3 * 256 - 2 * f) >> 16;
}

// Bilinearly interpolated offset from one wave grid, roughly -256..256.
static int enhance_flicker_at(const int (*wave)[FLICKER_GRID], int tile)
{
    int x = tile % HEX_GRID_WIDTH;
    int y = tile / HEX_GRID_WIDTH;

    int mask = (1 << FLICKER_CELL_SHIFT) - 1;
    int fx = enhance_smooth_frac((x & mask) << (8 - FLICKER_CELL_SHIFT));
    int fy = enhance_smooth_frac((y & mask) << (8 - FLICKER_CELL_SHIFT));

    int cx = (x >> FLICKER_CELL_SHIFT) & (FLICKER_GRID - 1);
    int cy = (y >> FLICKER_CELL_SHIFT) & (FLICKER_GRID - 1);
    int cx1 = (cx + 1) & (FLICKER_GRID - 1);
    int cy1 = (cy + 1) & (FLICKER_GRID - 1);

    int top = wave[cy][cx] + ((wave[cy][cx1] - wave[cy][cx]) * fx >> 8);
    int bottom = wave[cy1][cx] + ((wave[cy1][cx1] - wave[cy1][cx]) * fx >> 8);
    return top + ((bottom - top) * fy >> 8);
}

// Flicker offset for a tile, roughly -256..256. Tiles lit by warm sources
// (fires, braziers) get the fast wave; everything else - lamps, and daylight
// coming in from outside - gets the slow one, with a blend in between so a
// tile lit by both doesn't snap between the two.
static int enhance_flicker_value(int elevation, int tile)
{
    int slow = enhance_flicker_at(flicker_slow, tile);

    if (!elevationIsValid(elevation) || !hexGridTileIsValid(tile)) {
        return slow;
    }

    int accum = warm_accum[elevation][tile];
    if (accum <= 0) {
        return slow;
    }

    // 655 is the empty-tile base, matching enhance_light_table.
    int sourceLight = light_get_tile_true(elevation, tile) - 655;
    if (sourceLight < 1024) {
        return slow;
    }

    int warmth = std::clamp(accum * 256 / sourceLight, 0, 256);
    int fast = enhance_flicker_at(flicker_fast, tile);
    return slow + ((fast - slow) * warmth >> 8);
}

static void enhance_update_flicker(unsigned int now)
{
    if (!flicker_enabled) {
        return;
    }

    // Keep both waves slow enough that the ~15 fps refresh below samples them
    // many times per cycle - faster waves land only a few samples per cycle
    // and the flicker reads as stepping rather than breathing. The slow wave
    // runs at a fifth of the flame rate, so steady light drifts rather than
    // flickers.
    double t = now / 1000.0;
    for (int cy = 0; cy < FLICKER_GRID; cy++) {
        for (int cx = 0; cx < FLICKER_GRID; cx++) {
            double phase = cx * 2.3 + cy * 3.7;
            double fast = sin(t * 5.5 + phase) * 0.55 + sin(t * 9.1 + phase * 1.9) * 0.45;
            double slow = sin(t * 1.1 + phase) * 0.55 + sin(t * 1.8 + phase * 1.9) * 0.45;
            flicker_fast[cy][cx] = (int)(fast * 256.0);
            flicker_slow[cy][cx] = (int)(slow * 256.0);
        }
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
    // The palette tint is applied to the whole display at present time, so it
    // cannot be confined to one half in SPLIT mode - only OLD suppresses it.
    if (compare_mode == ENHANCE_COMPARE_OLD) {
        colorSetDisplayTint(256, 256, 256);
        return;
    }

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
