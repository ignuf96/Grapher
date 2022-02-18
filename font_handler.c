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

#define MAX_FONTS 10

static bool fonts_loaded = false;
static TTF_Font *coordinate_font;

void font_init(void)
{
    TTF_Init();
    fonts_loaded = true;
    char* path = "OpenSans-Regular.ttf";
    coordinate_font = TTF_OpenFont(path, 24);

}

struct temp_font myfont;

struct temp_font load_string_font(SDL_Renderer* renderer, char* str)
{
    if(fonts_loaded)
    {
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

    }
    return myfont;
}

void font_cleanup(void)
{
    if(fonts_loaded)
    {
        TTF_CloseFont(coordinate_font);
    }
}
