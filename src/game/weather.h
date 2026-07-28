#ifndef FALLOUT_GAME_WEATHER_H_
#define FALLOUT_GAME_WEATHER_H_

#include "plib/gnw/rect.h"

namespace fallout {

// CE: Procedural weather (overcast/rain/storm, optional snow). The state
// machine is a deterministic function of game time, so it survives save/load
// without extra persistence. Particles and flashes are drawn palette-safe
// into the iso window buffer; ambience is conveyed via display tint and
// render-side light dimming only - no gameplay effect.

typedef enum WeatherState {
    WEATHER_STATE_CLEAR,
    WEATHER_STATE_OVERCAST,
    WEATHER_STATE_RAIN,
    WEATHER_STATE_STORM,
    WEATHER_STATE_SNOW,
} WeatherState;

int weather_init();
void weather_reset();
void weather_exit();

// Advances the simulation. Called from the enhancements background process.
void weather_update();

// 256 = no dimming. Applied to render-side light intensities.
int weather_light_dim();

// 256/256/256 = neutral. Multiplied into the display tint.
void weather_get_tint(int* r, int* g, int* b);

// True when particles or lightning need per-frame redraws.
bool weather_has_visuals();

void weather_render(unsigned char* buf, int pitch, Rect* rect, int bufWidth, int bufHeight);

} // namespace fallout

#endif /* FALLOUT_GAME_WEATHER_H_ */
