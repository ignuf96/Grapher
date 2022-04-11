#ifndef WORLD_H
#define WORLD_H

#include "../include/datatypes.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

struct WORLD {
    ivec2 world_dimensions;
    ivec2 screen_dimensions;
    ivec2 ASPECT_RATIO;
    ivec2 origin;
    ivec2 render_distance;
};


void init_world(SDL_Window *window);
void update_world(SDL_Event event);
ivec2 conv_units(int x, int y);
ivec2 conv_raw(int x, int y);
struct WORLD* get_world(void);
bool is_on_screen(SDL_Rect* rect);

#endif
