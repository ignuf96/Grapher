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
#include "../include/platform.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <SDL2/SDL_system.h>
#include <unistd.h>

static bool fonts_loaded = false;
TTF_Font *coordinate_font;

#define MAX_NUMBERS 10
char num_buffer[MAX_NUMBERS+1] = "0123456789";

#define PATH_LENGTH 255

SPRITE p_font[MAX_FONT_SIZE];
SPRITE n_font[MAX_FONT_SIZE];


void font_init(SDL_Renderer* renderer)
{
    fonts_loaded = true;
    TTF_Init();

    coordinate_font = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", 35);
    if(!coordinate_font)
    {
        fprintf(stderr, "%s", "Font Path not found.\n");
    }
    SDL_Color text_color = {0, 255, 0, 255};

    for(int i=MIN_FONT_SIZE; i < MAX_FONT_SIZE; i++)
    {

        TTF_Font* font_path = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", i);
        if(!coordinate_font)
        {
            fprintf(stderr, "%s", "Font Path not found.\n");
        }
        SDL_Surface *positive_surface;
        SDL_Surface *negative_surface;

        positive_surface = TTF_RenderText_Solid(font_path, num_buffer, text_color);

        p_font[i].texture = SDL_CreateTextureFromSurface(renderer, positive_surface);
        p_font[i].rect.x = 0;
        p_font[i].rect.y = 0;
        p_font[i].rect.w = positive_surface->w;
        p_font[i].rect.h = positive_surface->h;

        negative_surface = TTF_RenderText_Solid(font_path, num_buffer, text_color);
        n_font[i].texture = SDL_CreateTextureFromSurface(renderer, negative_surface);
        n_font[i].rect.x = 0;
        n_font[i].rect.y = 0;
        n_font[i].rect.w = negative_surface->w;
        n_font[i].rect.h = negative_surface->h;

        SDL_FreeSurface(positive_surface);
        SDL_FreeSurface(negative_surface);

        TTF_CloseFont(font_path);
    }
}

SPRITE* load_texture(int sign, int size)
{

    SPRITE* req_font;
    if(sign >= 0)
    {
        req_font = &(p_font[size]);
    } else if(sign < 0)
    {
        req_font = &(n_font[size]);
    } else req_font = &(p_font[size]);

    return req_font;
}

void font_cleanup(void)
{
    if(fonts_loaded)
    {
        //Causes seg fault
        //TTF_CloseFont(coordinate_font);
        for(int i=MIN_FONT_SIZE; i < MAX_FONT_NUMBERS; i++)
        {
            SDL_DestroyTexture(p_font[i].texture);
            SDL_DestroyTexture(n_font[i].texture);
        }
    }
}
