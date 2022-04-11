#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define MAX_FONT_NUMBERS 300
#define MIN_FONT_SIZE 20
#define MAX_FONT_SIZE 150
#define FONT_SIZE_RANGE MAX_FONT_SIZE - MIN_FONT_SIZE

typedef struct SPRITE {
    SDL_Rect rect;
    SDL_Texture* texture;

}SPRITE;

SPRITE* load_texture(int font_size);
SPRITE* load_sign(int size);
void font_init(SDL_Renderer* renderer);
void font_cleanup(void);

#endif
