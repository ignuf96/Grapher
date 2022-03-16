#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define MAX_FONT_NUMBERS 300
#define MIN_FONT_SIZE 28
#define MAX_FONT_SIZE 36
#define FONT_SIZE_RANGE MAX_FONT_SIZE - MIN_FONT_SIZE

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
void load_string_font(SDL_Renderer* renderer, SPRITE* sprite, SDL_Texture* texture, char* str);
void font_init(SDL_Renderer* renderer);
void font_cleanup(void);

#endif
