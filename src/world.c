#include "../include/world.h"
#include "../include/datatypes.h"
#include <SDL2/SDL_events.h>

typedef struct WORLD {
    ivec2 world_dimensions;
    ivec2 screen_dimensions;
    ivec2 ASPECT_RATIO;
    ivec2 origin;
    ivec2 render_distance;
}myworld;

static myworld world;

void init_world(SDL_Window *window)
{
    SDL_GetWindowSize(window, &world.screen_dimensions.x, &world.screen_dimensions.y);
    world.ASPECT_RATIO.x = 16;
    world.ASPECT_RATIO.y = 9;
    world.world_dimensions.x = world.screen_dimensions.x/world.ASPECT_RATIO.x;
    world.world_dimensions.y = world.screen_dimensions.y/world.ASPECT_RATIO.y;

    world.origin.x = world.screen_dimensions.x/2;
    world.origin.y = world.screen_dimensions.y/2;
    world.render_distance.x = world.screen_dimensions.x;
    world.render_distance.y = world.screen_dimensions.y;

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

                    printf("Height: %d\nWidth: %d\n", event.window.data2, event.window.data1);

                    world.world_dimensions.x = world.screen_dimensions.x/world.ASPECT_RATIO.x;
                    world.world_dimensions.y = world.screen_dimensions.y/world.ASPECT_RATIO.y;

                    world.origin.x = world.screen_dimensions.x/2;
                    world.origin.y = world.screen_dimensions.y/2;
                    world.render_distance.x = world.screen_dimensions.x;
                    world.render_distance.y = world.screen_dimensions.y;
            }
            break;
    }
}

ivec2 get_screen(void)
{
    return world.screen_dimensions;
}

ivec2 get_world(void)
{
    return world.world_dimensions;
}

ivec2 get_render_distance(void)
{
    return world.render_distance;
}

ivec2 get_origin(void)
{
    return world.origin;
}
