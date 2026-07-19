#include "game/weather.h"

#include <string.h>

#include "game/art.h"
#include "game/config.h"
#include "game/gconfig.h"
#include "game/map.h"
#include "game/object.h"
#include "game/scripts.h"
#include "game/tile.h"
#include "platform_compat.h"
#include "plib/color/color.h"
#include "plib/gnw/input.h"

namespace fallout {

#define WEATHER_MAX_PARTICLES 700

// One weather slot lasts 4 game hours (1 game hour = 36000 ticks).
#define WEATHER_SLOT_TICKS (4 * 36000)

// Milliseconds to fade weather intensity from 0 to full.
#define WEATHER_FADE_DURATION 12000

#define WEATHER_FLASH_DURATION 260

typedef struct WeatherParticle {
    // 16.16 fixed point window coordinates.
    int x;
    int y;
    int vx;
    int vy;
} WeatherParticle;

typedef struct WeatherStateProfile {
    int dim; // 256 = no dimming
    int tintR;
    int tintG;
    int tintB;
    int particleTarget;
} WeatherStateProfile;

static int weather_pick_state(int slot);
static void weather_check_outdoor();
static bool weather_map_name_in_list(const char* list);
static bool weather_dude_under_roof();
static int weather_roll(int max);
static void weather_spawn_particle(WeatherParticle* particle, bool anywhere);

static const WeatherStateProfile weather_profiles[] = {
    /* CLEAR */ { 256, 256, 256, 256, 0 },
    /* OVERCAST */ { 244, 243, 245, 250, 0 },
    /* RAIN */ { 228, 232, 238, 248, 500 },
    /* STORM */ { 205, 216, 224, 242, 650 },
    /* SNOW */ { 240, 248, 250, 255, 350 },
};

static bool weather_enabled = false;
static bool weather_snow_enabled = false;
static char weather_outdoor_maps[256];
static char weather_indoor_maps[256];

static int weather_state = WEATHER_STATE_CLEAR;
static int weather_slot = -1;
static int weather_intensity = 0; // 0..256
static bool weather_outdoor = false;
static unsigned int weather_last_update;
static unsigned int weather_last_outdoor_check;
static unsigned int weather_flash_started;
static bool weather_flash_active = false;
static int weather_wind = 0; // pixels per second
static unsigned int weather_rng = 0x2E1A31C7;

static WeatherParticle weather_particles[WEATHER_MAX_PARTICLES];
static int weather_particle_count = 0;
static int weather_area_width = 640;
static int weather_area_height = 380;

// 0 = uninitialized so first update spawns from scratch.
static bool weather_initialized = false;

int weather_init()
{
    int value;

    weather_enabled = true;
    if (config_get_value(&game_config, "enhancements", "weather", &value)) {
        weather_enabled = value != 0;
    }

    weather_snow_enabled = false;
    if (config_get_value(&game_config, "enhancements", "weather_snow", &value)) {
        weather_snow_enabled = value != 0;
    }

    weather_outdoor_maps[0] = '\0';
    weather_indoor_maps[0] = '\0';

    char* list;
    if (config_get_string(&game_config, "enhancements", "weather_outdoor_maps", &list) && list != NULL) {
        strncpy(weather_outdoor_maps, list, sizeof(weather_outdoor_maps) - 1);
    }
    if (config_get_string(&game_config, "enhancements", "weather_indoor_maps", &list) && list != NULL) {
        strncpy(weather_indoor_maps, list, sizeof(weather_indoor_maps) - 1);
    }

    weather_reset();
    weather_initialized = true;

    return 0;
}

void weather_reset()
{
    weather_state = WEATHER_STATE_CLEAR;
    weather_slot = -1;
    weather_intensity = 0;
    weather_outdoor = false;
    weather_last_update = get_time();
    weather_last_outdoor_check = 0;
    weather_flash_active = false;
    weather_particle_count = 0;
}

void weather_exit()
{
    weather_initialized = false;
}

void weather_update()
{
    if (!weather_initialized || !weather_enabled) {
        return;
    }

    unsigned int now = get_time();
    unsigned int dt = elapsed_tocks(now, weather_last_update);
    weather_last_update = now;
    if (dt > 250) {
        dt = 250;
    }

    int slot = game_time() / WEATHER_SLOT_TICKS;
    if (slot != weather_slot) {
        weather_slot = slot;
        weather_state = weather_pick_state(slot);
    }

    if (elapsed_tocks(now, weather_last_outdoor_check) >= 500) {
        weather_last_outdoor_check = now;
        weather_check_outdoor();
    }

    // Fade intensity toward the target. Indoors (or clear weather) fades out.
    int target = weather_outdoor && weather_state != WEATHER_STATE_CLEAR ? 256 : 0;
    int step = (int)(dt * 256 / WEATHER_FADE_DURATION);
    if (step < 1) {
        step = 1;
    }
    if (weather_intensity < target) {
        weather_intensity = weather_intensity + step > target ? target : weather_intensity + step;
    } else if (weather_intensity > target) {
        weather_intensity = weather_intensity - step < target ? target : weather_intensity - step;
    }

    const WeatherStateProfile* profile = &(weather_profiles[weather_state]);

    // Wind sways slowly; storms are gustier.
    int windBase = weather_state == WEATHER_STATE_STORM ? 130 : 45;
    int phase = (int)(now / 64 % 6283);
    // Cheap sine approximation via triangle wave is enough for wind sway.
    int tri = phase < 3141 ? phase - 1570 : 4712 - phase;
    weather_wind = windBase * tri / 1571;

    // Lightning.
    if (weather_flash_active && elapsed_tocks(now, weather_flash_started) >= WEATHER_FLASH_DURATION) {
        weather_flash_active = false;
    }
    if (weather_state == WEATHER_STATE_STORM && weather_intensity > 200 && !weather_flash_active) {
        // Roughly one flash every ~9 seconds of storm.
        if (weather_roll(9000) < (int)dt) {
            weather_flash_active = true;
            weather_flash_started = now;
        }
    }

    // Particles.
    int particleTarget = profile->particleTarget * weather_intensity / 256;
    if (particleTarget > WEATHER_MAX_PARTICLES) {
        particleTarget = WEATHER_MAX_PARTICLES;
    }

    while (weather_particle_count < particleTarget) {
        weather_spawn_particle(&(weather_particles[weather_particle_count]), true);
        weather_particle_count++;
    }
    if (weather_particle_count > particleTarget) {
        weather_particle_count = particleTarget;
    }

    bool snow = weather_state == WEATHER_STATE_SNOW;
    int windVx = weather_wind << 16;
    for (int index = 0; index < weather_particle_count; index++) {
        WeatherParticle* particle = &(weather_particles[index]);
        particle->x += (int)((long long)(particle->vx + windVx) * dt / 1000);
        particle->y += (int)((long long)particle->vy * dt / 1000);

        if (snow) {
            // Slight per-particle horizontal wobble.
            particle->x += (weather_roll(3) - 1) << 14;
        }

        int x = particle->x >> 16;
        int y = particle->y >> 16;
        if (y > weather_area_height || x < -16 || x > weather_area_width + 16) {
            weather_spawn_particle(particle, false);
        }
    }
}

int weather_light_dim()
{
    if (!weather_initialized || weather_intensity == 0) {
        return 256;
    }

    const WeatherStateProfile* profile = &(weather_profiles[weather_state]);
    return 256 + (profile->dim - 256) * weather_intensity / 256;
}

void weather_get_tint(int* r, int* g, int* b)
{
    *r = 256;
    *g = 256;
    *b = 256;

    if (!weather_initialized || weather_intensity == 0) {
        return;
    }

    const WeatherStateProfile* profile = &(weather_profiles[weather_state]);
    *r = 256 + (profile->tintR - 256) * weather_intensity / 256;
    *g = 256 + (profile->tintG - 256) * weather_intensity / 256;
    *b = 256 + (profile->tintB - 256) * weather_intensity / 256;
}

bool weather_has_visuals()
{
    if (!weather_initialized || !weather_enabled) {
        return false;
    }

    if (weather_flash_active) {
        return true;
    }

    return weather_particle_count > 0 && weather_intensity > 8;
}

void weather_render(unsigned char* buf, int pitch, Rect* rect, int bufWidth, int bufHeight)
{
    if (!weather_has_visuals()) {
        return;
    }

    weather_area_width = bufWidth;
    weather_area_height = bufHeight;

    bool snow = weather_state == WEATHER_STATE_SNOW;
    int slant = weather_wind / 24;
    if (slant < -3) {
        slant = -3;
    }
    if (slant > 3) {
        slant = 3;
    }

    for (int index = 0; index < weather_particle_count; index++) {
        WeatherParticle* particle = &(weather_particles[index]);
        int x = particle->x >> 16;
        int y = particle->y >> 16;

        if (snow) {
            if (x >= rect->ulx && x <= rect->lrx && y >= rect->uly && y <= rect->lry) {
                unsigned char* p = buf + pitch * y + x;
                if (*p < 0xE5) {
                    *p = intensityColorTable[*p][170];
                }
            }
        } else {
            // A short slanted streak, brighter at the head.
            for (int segment = 0; segment < 3; segment++) {
                int sx = x + slant * segment / 3;
                int sy = y - segment * 2;
                if (sx < rect->ulx || sx > rect->lrx || sy < rect->uly || sy > rect->lry) {
                    continue;
                }
                unsigned char* p = buf + pitch * sy + sx;
                if (*p < 0xE5) {
                    *p = intensityColorTable[*p][segment == 0 ? 152 : 142];
                }
            }
        }
    }

    if (weather_flash_active) {
        unsigned int age = elapsed_tocks(get_time(), weather_flash_started);
        if (age < WEATHER_FLASH_DURATION) {
            int strength = 52 * (WEATHER_FLASH_DURATION - (int)age) / WEATHER_FLASH_DURATION;
            if (strength > 0) {
                int lightenIndex = 128 + strength;
                for (int y = rect->uly; y <= rect->lry; y++) {
                    unsigned char* p = buf + pitch * y + rect->ulx;
                    for (int x = rect->ulx; x <= rect->lrx; x++) {
                        if (*p < 0xE5) {
                            *p = intensityColorTable[*p][lightenIndex];
                        }
                        p++;
                    }
                }
            }
        }
    }
}

static int weather_pick_state(int slot)
{
    unsigned int hash = (unsigned int)slot * 2654435761u;
    hash ^= hash >> 15;
    hash *= 2246822519u;
    hash ^= hash >> 13;

    int roll = (int)(hash % 100);

    int state;
    if (roll < 58) {
        state = WEATHER_STATE_CLEAR;
    } else if (roll < 78) {
        state = WEATHER_STATE_OVERCAST;
    } else if (roll < 93) {
        state = WEATHER_STATE_RAIN;
    } else {
        state = WEATHER_STATE_STORM;
    }

    if (weather_snow_enabled && (state == WEATHER_STATE_RAIN || state == WEATHER_STATE_STORM)) {
        state = WEATHER_STATE_SNOW;
    }

    return state;
}

static void weather_check_outdoor()
{
    weather_outdoor = false;

    if (obj_dude == NULL || obj_dude->tile == -1 || map_data.name[0] == '\0') {
        return;
    }

    if (weather_map_name_in_list(weather_indoor_maps)) {
        return;
    }

    if (!weather_map_name_in_list(weather_outdoor_maps)) {
        // Heuristic: weather only on ground level and not under a roof.
        if (map_elevation != 0) {
            return;
        }

        if (weather_dude_under_roof()) {
            return;
        }
    }

    weather_outdoor = true;
}

static bool weather_map_name_in_list(const char* list)
{
    if (list[0] == '\0') {
        return false;
    }

    char name[16];
    strncpy(name, map_data.name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    compat_strlwr(name);

    char lowerList[256];
    strncpy(lowerList, list, sizeof(lowerList) - 1);
    lowerList[sizeof(lowerList) - 1] = '\0';
    compat_strlwr(lowerList);

    char* token = lowerList;
    while (token != NULL && *token != '\0') {
        char* comma = strchr(token, ',');
        if (comma != NULL) {
            *comma = '\0';
        }

        while (*token == ' ') {
            token++;
        }

        if (*token != '\0' && strstr(name, token) != NULL) {
            return true;
        }

        token = comma != NULL ? comma + 1 : NULL;
    }

    return false;
}

static bool weather_dude_under_roof()
{
    int screenX;
    int screenY;
    if (tile_coord(obj_dude->tile, &screenX, &screenY, obj_dude->elevation) != 0) {
        return false;
    }

    screenX += 16;
    screenY += 8;

    int coordX;
    int coordY;
    square_xy_roof(screenX, screenY, obj_dude->elevation, &coordX, &coordY);
    if (coordX < 0 || coordX >= SQUARE_GRID_WIDTH || coordY < 0 || coordY >= SQUARE_GRID_HEIGHT) {
        return false;
    }

    int upper = square[obj_dude->elevation]->field_0[SQUARE_GRID_WIDTH * coordY + coordX] >> 16;
    if ((((upper & 0xF000) >> 12) & 0x01) != 0) {
        return false;
    }

    return art_id(OBJ_TYPE_TILE, upper & 0xFFF, 0, 0, 0) != art_id(OBJ_TYPE_TILE, 1, 0, 0, 0);
}

static int weather_roll(int max)
{
    weather_rng = weather_rng * 1664525 + 1013904223;
    return (int)((weather_rng >> 16) % (unsigned int)max);
}

static void weather_spawn_particle(WeatherParticle* particle, bool anywhere)
{
    bool snow = weather_state == WEATHER_STATE_SNOW;

    particle->x = (weather_roll(weather_area_width + 32) - 16) << 16;
    if (anywhere) {
        particle->y = weather_roll(weather_area_height) << 16;
    } else {
        particle->y = -(weather_roll(24) << 16);
    }

    if (snow) {
        particle->vx = (weather_roll(30) - 15) << 16;
        particle->vy = (40 + weather_roll(35)) << 16;
    } else {
        particle->vx = 0;
        int speed = weather_state == WEATHER_STATE_STORM ? 700 : 520;
        particle->vy = (speed + weather_roll(220)) << 16;
    }
}

} // namespace fallout
