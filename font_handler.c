#include "font_handler.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_FONTS  62
#define FONT_SIZE 24
#define FONT_SIZES 64

// Constants used to access array of the alphabet and numbers 0-9
// Ugly and verbose. Fix later
const int
    LETTER_A = 0,
    LETTER_B = 1,
    LETTER_C = 2,
    LETTER_D = 3,
    LETTER_E = 4,
    LETTER_F = 5,
    LETTER_G = 6,
    LETTER_H = 7,
    LETTER_I = 8,
    LETTER_J = 9,
    LETTER_K = 10,
    LETTER_L = 11,
    LETTER_M = 12,
    LETTER_N = 13,
    LETTER_O = 14,
    LETTER_P = 15,
    LETTER_Q = 16,
    LETTER_R = 17,
    LETTER_S = 18,
    LETTER_T = 19,
    LETTER_U = 20,
    LETTER_V = 21,
    LETTER_W = 22,
    LETTER_X = 23,
    LETTER_Y = 24,
    LETTER_Z = 25,

    UPPER_LETTER_A = 26,
    UPPER_LETTER_B = 27,
    UPPER_LETTER_C = 28,
    UPPER_LETTER_D = 29,
    UPPER_LETTER_E = 30,
    UPPER_LETTER_F = 31,
    UPPER_LETTER_G = 32,
    UPPER_LETTER_H = 33,
    UPPER_LETTER_I = 34,
    UPPER_LETTER_J = 35,
    UPPER_LETTER_K = 36,
    UPPER_LETTER_L = 37,
    UPPER_LETTER_M = 38,
    UPPER_LETTER_N = 39,
    UPPER_LETTER_O = 40,
    UPPER_LETTER_P = 41,
    UPPER_LETTER_Q = 42,
    UPPER_LETTER_R = 43,
    UPPER_LETTER_S = 44,
    UPPER_LETTER_T = 45,
    UPPER_LETTER_U = 46,
    UPPER_LETTER_V = 47,
    UPPER_LETTER_W = 48,
    UPPER_LETTER_X = 49,
    UPPER_LETTER_Y = 50,
    UPPER_LETTER_Z = 51,

    NUMBER_0 = 52,
    NUMBER_1 = 53,
    NUMBER_2 = 54,
    NUMBER_3 = 55,
    NUMBER_4 = 56,
    NUMBER_5 = 57,
    NUMBER_6 = 58,
    NUMBER_7 = 59,
    NUMBER_8 = 60,
    NUMBER_9 = 61;

const char alphanum[MAX_FONTS][2] = {
{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}, {"j"}, {"k"}, {"l"}, {"m"},
{"n"}, {"o"}, {"p"}, {"q"}, {"r"}, {"s"}, {"t"}, {"u"}, {"v"}, {"w"}, {"x"}, {"y"}, {"z"},

{"A"}, {"B"}, {"C"}, {"D"}, {"E"}, {"F"}, {"G"}, {"H"}, {"I"}, {"J"}, {"K"}, {"L"}, {"M"},
{"N"}, {"O"}, {"P"}, {"Q"}, {"R"}, {"S"}, {"T"}, {"U"}, {"V"}, {"W"}, {"X"}, {"Y"}, {"Z"},
{"0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"},
};

static SDL_Texture *font_textures[FONT_SIZES][MAX_FONTS];
static bool fonts_loaded = false;

SDL_Rect rect[FONT_SIZES][MAX_FONTS];

void font_init(void)
{
   TTF_Init();
}

void load_fonts(SDL_Renderer *renderer, const char *path)
{
    if(!fonts_loaded)
    {
        //TTF_Font *font = TTF_OpenFont(path, FONT_SIZE);
        for(int i = 0; i < FONT_SIZES; i++)
        {
            TTF_Font *font = TTF_OpenFont(path, i);
            for(int j=0; j < MAX_FONTS; j++)
            {
            SDL_Surface *surface;
            SDL_Color textColor = { 0, 255, 0, 255};

            surface = TTF_RenderText_Solid(font, alphanum[i], textColor);
            font_textures[i][j] = SDL_CreateTextureFromSurface(renderer, surface);

            rect[i][j].x = 0;
            rect[i][j].y = 0;
            rect[i][j].w = surface->w;
            rect[i][j].h = surface->h;

            SDL_FreeSurface(surface);

            //TTF_CloseFont(font);
            }
        }
        fonts_loaded = true;
    }

}

//struct temp_font myfont;


struct temp_font load_string_font(SDL_Renderer* renderer, const char *path, char* str)
{
    struct temp_font myfont;

    if(fonts_loaded)
    {
        TTF_Font *font = TTF_OpenFont(path, 24);
        SDL_Surface *surface;
        SDL_Color textColor = { 0, 255, 0, 255};

        surface = TTF_RenderText_Solid(font, str, textColor);
        myfont.texture = SDL_CreateTextureFromSurface(renderer, surface);

        myfont.rect.x = 0;
        myfont.rect.y = 0;
        myfont.rect.w = surface->w;
        myfont.rect.h = surface->h;

        SDL_FreeSurface(surface);
        TTF_CloseFont(font);

    }
    return myfont;
}

SDL_Texture* get_font_texture(int font_number, int font_size)
{
    // Does this cause a memory leak??
    SDL_Texture *temp_texture = NULL;

    if(fonts_loaded)
        temp_texture = font_textures[font_size][font_number];

    return temp_texture;
}

void font_cleanup(void)
{
    if(fonts_loaded)
    {
        for(int i = 0; i < FONT_SIZES; i++)
        {
            for(int j = 0; j < MAX_FONTS; j++)
            {
            SDL_DestroyTexture(font_textures[i][j]);
            }
        }
    }
}


//int allnumbers[FONT_SIZES][INT_MAX];

void make_all_numbers_to_fonts(void)
{

}

/* definitely change
int new_font_draw(SDL_Renderer* renderer, SDL_Texture texture, int x, int y)
{
   //SDL_RenderCopy(renderer, texture);
}
*/

int font_draw(SDL_Renderer *renderer, int font_num, int font_size, int x, int y)
{
    int error = 0;

    if(font_size > FONT_SIZES | font_size < 0)
    {
        error = 1;
    } else {

    rect[font_size][font_num].x = x;
    rect[font_size][font_num].y = y;
    SDL_RenderCopy(renderer, get_font_texture(font_num, font_size), NULL, &rect[font_size][font_num]);
    }

    return error;
}
/* Do this later, too lazy right now, making a shit ton of fonts for now, removing later(hopefully) */
/*
SDL_Texture* conv_num_to_font(int number, int font_size)
{
    if(number < 10 && number >= 0)
    {

    }
    int divisor = 10;
    int result = number;

    do
    {

    }


    for(int divisor = 10; (divisor )
}
*/
