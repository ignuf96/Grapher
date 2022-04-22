/* PURPOSE:
** Library with helper functions that convert any string into a texture.
**
** TODO:
** 1. Make library convert to any font-type and any size and any color.
** 2. Container for the storage of multiple fonts.
** 3. Cleanup function to alleviate the burden of cleaning up dynamic storage from caller. Will cleanup container and cycle through font container.
**
**
 */

#include "../include/font_handler.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <SDL2/SDL_system.h>

static bool fonts_loaded = false;
TTF_Font *coordinate_font;

#define MAX_NUMBERS 10
#define MAX_SIGNS 1
char num_buffer[MAX_NUMBERS+1] = "0123456789";
char negative_sign_buffer[MAX_SIGNS] = "-";

#define PATH_LENGTH 255

SPRITE number_font[MAX_FONT_SIZE];
SPRITE negative_sign_font[MAX_FONT_SIZE];

void font_init(SDL_Renderer* renderer)
{
    fonts_loaded = true;
    TTF_Init();

    coordinate_font = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", 35);
    if(!coordinate_font)
    {
        fprintf(stderr, "%s", "Font Path not found.\n");
    }
    SDL_Color text_color = {255, 0, 0, 255};

    for(int i=MIN_FONT_SIZE; i < MAX_FONT_SIZE; i++)
    {

        TTF_Font* font_path = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", i);
        if(!coordinate_font)
        {
            fprintf(stderr, "%s", "Font Path not found.\n");
        }
        SDL_Surface *number_surface;
        SDL_Surface *negative_sign_surface;

        number_surface = TTF_RenderText_Solid(font_path, num_buffer, text_color);

        number_font[i].texture = SDL_CreateTextureFromSurface(renderer, number_surface);
        number_font[i].rect.x = 0;
        number_font[i].rect.y = 0;
        number_font[i].rect.w = number_surface->w;
        number_font[i].rect.h = number_surface->h;

        negative_sign_surface = TTF_RenderText_Solid(font_path, negative_sign_buffer, text_color);
        negative_sign_font[i].texture = SDL_CreateTextureFromSurface(renderer, negative_sign_surface);
        negative_sign_font[i].rect.x = 0;
        negative_sign_font[i].rect.y = 0;
        negative_sign_font[i].rect.w = negative_sign_surface->w;
        negative_sign_font[i].rect.h = negative_sign_surface->h;

        SDL_FreeSurface(number_surface);
        SDL_FreeSurface(negative_sign_surface);

        TTF_CloseFont(font_path);
    }
}

SPRITE* load_texture(int size)
{
    return &(number_font[size]);
}

SPRITE* load_sign(int size)
{
    return &(negative_sign_font[size]) ;
}

void font_cleanup(void)
{
    if(fonts_loaded)
    {
        //Causes seg fault
        //TTF_CloseFont(coordinate_font);
        for(int i=MIN_FONT_SIZE; i < MAX_FONT_SIZE; i++)
        {
            SDL_DestroyTexture(number_font[i].texture);
            SDL_DestroyTexture(negative_sign_font[i].texture);
        }
    }
}
