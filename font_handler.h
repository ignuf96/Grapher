#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

struct temp_font {
    SDL_Rect rect;
    SDL_Texture* texture;

};

extern const int
    LETTER_A,
    LETTER_B,
    LETTER_C,
    LETTER_D,
    LETTER_E,
    LETTER_F,
    LETTER_G,
    LETTER_H,
    LETTER_I,
    LETTER_J,
    LETTER_K,
    LETTER_L,
    LETTER_M,
    LETTER_N,
    LETTER_O,
    LETTER_P,
    LETTER_Q,
    LETTER_R,
    LETTER_S,
    LETTER_T,
    LETTER_U,
    LETTER_V,
    LETTER_W,
    LETTER_X,
    LETTER_Y,
    LETTER_Z,


    UPPER_LETTER_A,
    UPPER_LETTER_B,
    UPPER_LETTER_C,
    UPPER_LETTER_D,
    UPPER_LETTER_E,
    UPPER_LETTER_F,
    UPPER_LETTER_G,
    UPPER_LETTER_H,
    UPPER_LETTER_I,
    UPPER_LETTER_J,
    UPPER_LETTER_K,
    UPPER_LETTER_L,
    UPPER_LETTER_M,
    UPPER_LETTER_N,
    UPPER_LETTER_O,
    UPPER_LETTER_P,
    UPPER_LETTER_Q,
    UPPER_LETTER_R,
    UPPER_LETTER_S,
    UPPER_LETTER_T,
    UPPER_LETTER_U,
    UPPER_LETTER_V,
    UPPER_LETTER_W,
    UPPER_LETTER_X,
    UPPER_LETTER_Y,
    UPPER_LETTER_Z,

    NUMBER_0,
    NUMBER_1,
    NUMBER_2,
    NUMBER_3,
    NUMBER_4,
    NUMBER_5,
    NUMBER_6,
    NUMBER_7,
    NUMBER_8,
    NUMBER_9;

void font_init(void);

/* Get path of font and array string of text to represent each texture
 * Job is to load every single one  */
void load_fonts(SDL_Renderer *renderer, const char *path);

struct temp_font load_string_font(SDL_Renderer* renderer, const char* path, char* str);

SDL_Texture* get_font_texture(int font_number, int font_size);

int font_draw(SDL_Renderer *renderer, int font_num, int font_size, int x, int y);

void font_cleanup(void);

SDL_Texture* conv_num_to_font(int number, int font_size);

#endif
