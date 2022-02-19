#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

struct temp_font {
    SDL_Rect rect;
    SDL_Texture* texture;

};

void font_init(void);

struct temp_font load_string_font(SDL_Renderer* renderer, SDL_Texture* texture, char* str);

void font_cleanup(void);

#endif
