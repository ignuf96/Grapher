#include "../include/world.h"
#include "../include/datatypes.h"
#include <SDL2/SDL_events.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct WORLD myworld;
static myworld world;
static SDL_Rect window_rect;

void init_world(SDL_Window *window)
{
    SDL_GetWindowSize(window, &world.screen_dimensions.x, &world.screen_dimensions.y);
    window_rect.w = world.screen_dimensions.x;
    window_rect.h = world.screen_dimensions.y;
    window_rect.x = 0;
    window_rect.y = 0;

    printf("World dimensions\nw: %d\nh: %d\nx: %d\ny: %d\n", window_rect.w, window_rect.h, window_rect.x, window_rect.y);

    // Check if we are in landscape or portrait and adjust aspect ratio accordingly
    if(world.screen_dimensions.x > world.screen_dimensions.y)
    {
        world.ASPECT_RATIO.x = 16;
        world.ASPECT_RATIO.y = 9;
    } else {
        world.ASPECT_RATIO.x = 9;
        world.ASPECT_RATIO.y = 16;
    }

    world.world_dimensions.x = world.screen_dimensions.x/world.ASPECT_RATIO.x;
    world.world_dimensions.y = world.screen_dimensions.y/world.ASPECT_RATIO.y;

    world.origin.x = world.screen_dimensions.x/2;
    world.origin.y = world.screen_dimensions.y/2;
    world.render_distance.x = world.screen_dimensions.x;
    world.render_distance.y = world.screen_dimensions.y;

}

struct WORLD* get_world(void)
{
    return &world;
}

void update_world(SDL_Event event)
{
    switch (event.type)
    {
        case SDL_WINDOWEVENT:

            switch (event.window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    // Change line coordinates to new window size
                    world.screen_dimensions.x = event.window.data1;
                    world.screen_dimensions.y = event.window.data2;

                    // Check if we are in landscape or portrait and adjust aspect ratio accordingly
                    if(world.screen_dimensions.x > world.screen_dimensions.y)
                    {
                        world.ASPECT_RATIO.x = 16;
                        world.ASPECT_RATIO.y = 9;
                    } else {
                        world.ASPECT_RATIO.x = 9;
                        world.ASPECT_RATIO.y = 16;
                    }

                    printf("Height: %d\nWidth: %d\n", event.window.data2, event.window.data1);

                    world.world_dimensions.x = world.screen_dimensions.x/world.ASPECT_RATIO.x;
                    world.world_dimensions.y = world.screen_dimensions.y/world.ASPECT_RATIO.y;

                    world.origin.x = world.screen_dimensions.x/2;
                    world.origin.y = world.screen_dimensions.y/2;
                    world.render_distance.x = world.screen_dimensions.x;
                    world.render_distance.y = world.screen_dimensions.y;
                    window_rect.w = world.screen_dimensions.x;
                    window_rect.h = world.screen_dimensions.y;
            }
            break;
    }
}

bool is_on_screen(SDL_Rect* rect)
{
    bool intersected = false;

    if(SDL_HasIntersection(rect, &window_rect))
    {
        intersected = true;
    }

    return intersected;
}


ivec2 conv_units(int x, int y)
{
    struct INT_VECTOR2 temp;

    temp.x /= world.world_dimensions.x;
    temp.y /= world.world_dimensions.y;

    return temp;
}

ivec2 conv_raw(int x, int y)
{
    struct INT_VECTOR2 temp;

    temp.x *= (world.screen_dimensions.x/world.world_dimensions.x);
    temp.y *= (world.screen_dimensions.y/world.world_dimensions.y);

    return temp;
}
