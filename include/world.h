#ifndef WORLD_H
#define WORLD_H

#include "../include/datatypes.h"
#include <SDL2/SDL.h>

void init_world(SDL_Window *window);
void update_world(SDL_Event event);
ivec2 get_screen();
ivec2 get_world();
ivec2 get_render_distance(void);
ivec2 get_origin(void);

#endif
