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

#include "../include/font_handler.h"
#include "../include/platform.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <SDL2/SDL_system.h>
//#include <unistd.h>

#define MAX_FONTS 10
#define MAX_FONT_SIZES 22
//#define DEFAULT_FONTS_SIZE 2000

static bool fonts_loaded = false;
TTF_Font *coordinate_font;
char num_buffer[MAX_FONT_COORDINATES];

SPRITE p_font[MAX_FONT_COORDINATES][MAX_FONT_SIZES];
SPRITE n_font[MAX_FONT_COORDINATES][MAX_FONT_SIZES];

#define PATH_LENGTH 255
//static char* PROJECT_PATH;
static char coordinate_path_buffer[PATH_LENGTH];
static char number_path_buffer[PATH_LENGTH];

void font_init(SDL_Renderer* renderer)
{
    fonts_loaded = true;
    TTF_Init();
    printf("Init font_handler.c\n\n");

 /*PROJECT_PATH = SDL_GetBasePath();
    #ifdef __ANDROID__
    PROJECT_PATH = SDL_AndroidGetInternalStoragePath();
    #endif
    strncpy(coordinate_path_buffer, PROJECT_PATH, PATH_LENGTH-1);

 //   if(!(coordinate_font = TTF_OpenFont(strncat(coordinate_path_buffer, "assets/fonts/OpenSans-Regular.ttf", PATH_LENGTH-1), 35)))
 */
 coordinate_font = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", 35);
 /*   {
        printf("font path not found!!!\n:::%s", coordinate_path_buffer);
        printf("TTF ERROR: %s\n", TTF_GetError());
    }
    strncpy(number_path_buffer, PROJECT_PATH, PATH_LENGTH-1);
    strncat(number_path_buffer, "assets/fonts/OpenSans-Regular.ttf", PATH_LENGTH-1);*/
    SDL_Color text_color = {0, 255, 0, 255};
    for(int i=0; i < MAX_FONT_COORDINATES; i++)
    {
        for(int j=0; j < MAX_FONT_SIZES; j++)
        {

        //    TTF_Font* font_path = TTF_OpenFont(number_path_buffer, j);
        
        TTF_Font* font_path = TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", j);

             memset(&num_buffer[0], '\0', MAX_FONT_COORDINATES-1);
            SDL_Surface* positive_surface;
            SDL_Surface* negative_surface;

            snprintf(num_buffer, MAX_FONT_COORDINATES-1, "%d", i);

            if(!(positive_surface = TTF_RenderText_Solid(font_path, num_buffer, text_color))){
                printf("Error creating surface from text");
              
            }
          
            p_font[i][j].texture = SDL_CreateTextureFromSurface(renderer, positive_surface);
            p_font[i][j].rect.x = 0;
            p_font[i][j].rect.y = 0;
            p_font[i][j].rect.w = positive_surface->w;
            p_font[i][j].rect.h = positive_surface->h;
            //memset(&num_buffer[0], '\0', DEFAULT_FONTS_SIZE-1);

            snprintf(num_buffer, MAX_FONT_COORDINATES-1, "%d", -i);
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
            //TTF_CloseFont(coordinate_font);

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


//Might cause a memory leak? because texture remains??
SPRITE* load_string_font(SDL_Renderer* renderer, SDL_Texture* texture, char* str)
{
    static SPRITE myfont;
    //if(fonts_loaded)
    //{
        myfont.texture = texture;
        SDL_Surface *surface;
        SDL_Color textColor = { 255, 0, 0, 255};

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
       // TTF_CloseFont(coordinate_font);
    }
    for(int i=0; i < MAX_FONT_COORDINATES; i++)
    {
        for(int j=0; j < MAX_FONT_SIZES; j++)
        {
            SDL_DestroyTexture(p_font[i][j].texture);
            SDL_DestroyTexture(n_font[i][j].texture);
        }
    }
}
