#ifndef FALLOUT_GAME_ENHANCE_H_
#define FALLOUT_GAME_ENHANCE_H_

#include "game/object_types.h"
#include "plib/gnw/rect.h"

namespace fallout {

// CE: Optional palette-safe visual enhancements (time-of-day grading, light
// flicker, projected critter shadows, weather, vignette). Everything resolves
// through the original 256-color lookup tables so the game keeps its
// prerendered look. All features are toggleable via the `[enhancements]`
// section of the game config.

int enhance_init();
void enhance_reset();
void enhance_exit();

// Invalidates per-map derived data (wall ambient occlusion). Called when a
// map finishes loading, after the object table is complete.
void enhance_map_changed();

bool enhance_shadows_enabled();

// Render-side light intensity for a tile: original max(ambient, tile light)
// plus flicker on above-ambient light sources and weather dimming. Only used
// by the renderer - gameplay light queries are untouched.
int enhance_render_light(int elevation, int tile);

// Render-side ambient intensity (weather dimming applied). Used for roofs.
int enhance_render_ambient();

// Draws a flattened, sheared silhouette of a critter frame onto the ground
// beneath it, darkening destination pixels through the intensity table.
void enhance_render_shadow(unsigned char* frameData, int frameWidth, int frameHeight,
    int sx, int sy, Rect* clipRect, unsigned char* dest, int destPitch, int light);

// Post-scene pass over the iso window buffer (weather overlay, vignette).
// Called at the end of the game refresh, right before the blit to screen.
void enhance_scene_post_process(unsigned char* buf, int pitch, Rect* rect, int bufWidth, int bufHeight);

// Colored light sources: a render-side per-tile "warmth" accumulator is kept
// in sync with the engine's tile light distribution. `obj_adjust_light` marks
// the current source before distributing (its warmth is derived from the
// source art's average palette color), and the tile light add/subtract
// primitives mirror every contribution here.
void enhance_light_source_begin(Object* obj);
void enhance_light_source_end();
void enhance_light_color_add(int elevation, int tile, int intensity);
void enhance_light_color_subtract(int elevation, int tile, int intensity);
void enhance_light_color_reset();

// Intensity lookup table to use when rendering pixels on this tile: the
// neutral `intensityColorTable` or a warm/cool tinted variant when the
// tile's visible light comes from a colored source.
unsigned char (*enhance_light_table(int elevation, int tile))[256];

} // namespace fallout

#endif /* FALLOUT_GAME_ENHANCE_H_ */
