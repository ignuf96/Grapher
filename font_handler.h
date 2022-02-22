#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define MAX_FONT_COORDINATES 3000

typedef struct loaded_font {
    SDL_Rect rect;
    SDL_Texture* texture;

}MY_FONT;

struct INT_VECTOR4 {
    int width;
    int height;
    int x;
    int y;
};

MY_FONT* load_number(int number);
MY_FONT* load_string_font(SDL_Renderer* renderer, SDL_Texture* texture, char* str);
void font_init(SDL_Renderer* renderer);
void font_cleanup(void);

#endif
