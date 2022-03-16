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
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <SDL2/SDL_system.h>
#include <unistd.h>

static bool fonts_loaded = false;
TTF_Font *coordinate_font;
char num_buffer[MAX_FONT_NUMBERS+1];

#define PATH_LENGTH 255

SPRITE p_font[MAX_FONT_NUMBERS+1][FONT_SIZE_RANGE+1];
SPRITE n_font[MAX_FONT_NUMBERS+1][FONT_SIZE_RANGE+1];


void font_init(SDL_Renderer* renderer)
{
    fonts_loaded = true;
    TTF_Init();

    coordinate_font = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", 35);
    SDL_Color text_color = {0, 255, 0, 255};

    for(int i=0; i < MAX_FONT_NUMBERS; i++)
    {
        for(int j=MIN_FONT_SIZE; j < MAX_FONT_SIZE; j++)
        {
            TTF_Font* font_path = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", j);
            memset(&num_buffer[0], '\0', MAX_FONT_NUMBERS);
            SDL_Surface* positive_surface;
            SDL_Surface* negative_surface;

            snprintf(num_buffer, MAX_FONT_NUMBERS, "%d", i);

            positive_surface = TTF_RenderText_Solid(font_path, num_buffer, text_color);

            p_font[i][j].texture = SDL_CreateTextureFromSurface(renderer, positive_surface);
            p_font[i][j].rect.x = 0;
            p_font[i][j].rect.y = 0;
            p_font[i][j].rect.w = positive_surface->w;
            p_font[i][j].rect.h = positive_surface->h;

            snprintf(num_buffer, MAX_FONT_NUMBERS, "%d", -i);
            negative_surface = TTF_RenderText_Solid(font_path, num_buffer, text_color);
            if(!(n_font[i][j].texture = SDL_CreateTextureFromSurface(renderer, negative_surface)))
            {
                printf("Error creating texture\n");
            }
            n_font[i][j].rect.x = 0;
            n_font[i][j].rect.y = 0;
            n_font[i][j].rect.w = negative_surface->w;
            n_font[i][j].rect.h = negative_surface->h;

            SDL_FreeSurface(positive_surface);
            SDL_FreeSurface(negative_surface);

            TTF_CloseFont(font_path);
        }
    }
}

SPRITE* load_number(int number, int font_size)
{
    SPRITE* req_font;
    if(number >= 0)
    {
        req_font = &(p_font[number][font_size]);
    } else if(number < 0)
    {
        req_font = &(n_font[-number][font_size]);
    } else req_font = &(p_font[0][font_size]);

    return req_font;
}

void load_string_font(SDL_Renderer* renderer, SPRITE* sprite, SDL_Texture* texture, char* str)
{
    if(fonts_loaded)
    {
        sprite->texture = texture;
        SDL_Surface *surface;
        SDL_Color textColor = { 255, 0, 0, 255};

        surface = TTF_RenderText_Solid(coordinate_font, str, textColor);
        sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);

        sprite->rect.x = 0;
        sprite->rect.y = 0;
        sprite->rect.w = surface->w;
        sprite->rect.h = surface->h;

        SDL_FreeSurface(surface);

    }
}
void font_cleanup(void)
{
    if(fonts_loaded)
    {
        //Causes seg fault
        //TTF_CloseFont(coordinate_font);
        for(int i=0; i < MAX_FONT_NUMBERS; i++)
        {
            for(int j=MIN_FONT_SIZE; j < MAX_FONT_SIZE; j++)
            {
                SDL_DestroyTexture(p_font[i][j].texture);
                SDL_DestroyTexture(n_font[i][j].texture);
            }
        }
    }
}
