#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define MAX_FONT_COORDINATES 30

typedef struct SPRITE {
    SDL_Rect rect;
    SDL_Texture* texture;

}SPRITE;

struct INT_VECTOR4 {
    int width;
    int height;
    int x;
    int y;
};

SPRITE* load_number(int number, int font_size);
SPRITE* load_string_font(SDL_Renderer* renderer, SDL_Texture* texture, char* str);
void font_init(SDL_Renderer* renderer);
void font_cleanup(void);

#endif
