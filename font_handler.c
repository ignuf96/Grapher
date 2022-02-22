/* PURPOSE:
** Library with helper functions that convert any string into a texture.
**
** TODO:
** 1. Make library convert to any font-type and any size and any color.
** 2. Container for the storage of multiple fonts.
** 3. Cleanup function to aleviate the burden of cleaning up dynamic storage from caller. Will cleanup container and cycle through font container.
**
**
 */

#include "font_handler.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>

#define MAX_FONTS 10
//#define DEFAULT_FONTS_SIZE 2000

static bool fonts_loaded = false;
TTF_Font *coordinate_font;
char num_buffer[MAX_FONT_COORDINATES];

MY_FONT p_font[MAX_FONT_COORDINATES];
MY_FONT n_font[MAX_FONT_COORDINATES];

void font_init(SDL_Renderer* renderer)
{
    fonts_loaded = true;
    TTF_Init();
    printf("Init font_handler.c\n\n");

    if(!(coordinate_font = TTF_OpenFont("/home/johnny/Documents/Projects/SDL/Grapher/OpenSans-Regular.ttf", 24)))
    {
        printf("font path not found!!!\n");
        printf("TTF ERROR: %s\n", TTF_GetError());
    }
    SDL_Color text_color = {0, 255, 0, 255};

    for(int i=0; i < MAX_FONT_COORDINATES; i++)
    {
       // memset(&num_buffer[0], '\0', DEFAULT_FONTS_SIZE-1);
        SDL_Surface* positive_surface;
        SDL_Surface* negative_surface;

        snprintf(num_buffer, MAX_FONT_COORDINATES-1, "%d", i);

        if(!(positive_surface = TTF_RenderText_Solid(coordinate_font, num_buffer, text_color))){
            printf("Error creating surface from text");
            sleep(5);
        }
        p_font[i].texture = SDL_CreateTextureFromSurface(renderer, positive_surface);
        p_font[i].rect.x = 0;
        p_font[i].rect.y = 0;
        p_font[i].rect.w = positive_surface->w;
        p_font[i].rect.h = positive_surface->h;
        //memset(&num_buffer[0], '\0', DEFAULT_FONTS_SIZE-1);

        snprintf(num_buffer, MAX_FONT_COORDINATES-1, "%d", -i);
        negative_surface = TTF_RenderText_Solid(coordinate_font, num_buffer, text_color);
        if(!(n_font[i].texture = SDL_CreateTextureFromSurface(renderer, negative_surface)))
        {
            printf("Error creating texture\n");
            sleep(10);
        }
        n_font[i].rect.x = 0;
        n_font[i].rect.y = 0;
        n_font[i].rect.w = negative_surface->w;
        n_font[i].rect.h = negative_surface->h;

        SDL_FreeSurface(positive_surface);
        SDL_FreeSurface(negative_surface);

    }


}

struct loaded_font* load_number(int number)
{
    struct loaded_font* req_font;
    if(number >= 0)
    {
        req_font = &(p_font[number]);
    } else if(number < 0)
    {
        req_font = &(n_font[-number]);
    } else req_font = &(p_font[0]);

    return req_font;
}


//Might cause a memory leak? because texture remains??
MY_FONT* load_string_font(SDL_Renderer* renderer, SDL_Texture* texture, char* str)
{
    static MY_FONT myfont;
    //if(fonts_loaded)
    //{
        myfont.texture = texture;
        SDL_Surface *surface;
        SDL_Color textColor = { 0, 255, 0, 255};

        surface = TTF_RenderText_Solid(coordinate_font, str, textColor);
        myfont.texture = SDL_CreateTextureFromSurface(renderer, surface);

        myfont.rect.x = 0;
        myfont.rect.y = 0;
        myfont.rect.w = surface->w;
        myfont.rect.h = surface->h;

        //SDL_DestroyTexture(myfont.texture);
        SDL_FreeSurface(surface);

    //}
    return &myfont;
}

void font_cleanup(void)
{
    if(fonts_loaded)
    {
        //Cause seg fault
        //TTF_CloseFont(coordinate_font);
    }
    for(int i=0; i < MAX_FONT_COORDINATES; i++)
    {
        SDL_DestroyTexture(p_font[i].texture);
        SDL_DestroyTexture(n_font[i].texture);
    }
}
