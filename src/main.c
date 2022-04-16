/* TODO:
   1. Implement a constant frame-rate loop. Possibly 30-60 fps. This will help with smoothness of mouse gestures.
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_touch.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
/* MAX VALUE FOR GRAPH LONG_MAX */
#include <limits.h>
// For sleep();
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL_platform.h>
// Font things
#include "../include/font_handler.h"
#include "../include/datatypes.h"
#include "../include/world.h"

// Window information for initialization
#define WINDOW_TITLE "Grapher"
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED

// dimensions 16:9
static int axes_horizontal_line_width, axes_horizontal_line_height;
static int axes_vertical_line_width, axes_vertical_line_height;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Event e;

// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
static ivec2 origin_offset = { 0, 0 };
// Lower means faster
static ivec2 mouse_speed = { 30, 30 };
static ivec2 coordinate_location = { 0, 0 };
// Amount of space away from y and x axis
// Used for drawing lines in the quadrant
float spacing;
float starting_spacing;
float spacing_limit;

static float fps;
static const float FPS_TARGET = 60.0f;

static SPRITE axes_horizontal_line;
static SPRITE axes_vertical_line;

static enum DIRECTION{D_UP, D_DOWN, D_LEFT, D_RIGHT, D_NONE}direction=D_NONE;

static int mouse_x, mouse_y;

#define MAX_STR_BUFFER 100
static char str_buffer[100 - 1];
static char str_num_buffer[MAX_STR_BUFFER];

static bool quit = false;

// declarations
static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void update(void);
static void draw_coordinates(void);
static void draw_numbers(int position, int offset, enum DIRECTION direction);
static void create_sprite(SPRITE* sprite, char* path, int width, int height, int x, int y);
static void draw_mouse_coordinates(int x, int y);
static int get_number(int num, int part_size);
static void init_box(void);
static void get_quadrant_pos(int location, ivec2 *quadrant, ivec2 *distance);
static void draw_points(void);

SPRITE graph_box;
SPRITE dot_texture;
SPRITE background;

#define NUMBER_OF_QUADRANTS 4

// has to be same width and height or else causes bug
#define GRAPH_WIDTH 100
#define GRAPH_HEIGHT 100

SDL_Rect graph[NUMBER_OF_QUADRANTS][GRAPH_WIDTH][GRAPH_HEIGHT];
SDL_Rect *graphp = &graph[0][0][0];

struct POINTS {
	SDL_Rect rect;
	bool is_visible;
};

struct POINTS points[NUMBER_OF_QUADRANTS][GRAPH_WIDTH][GRAPH_HEIGHT] = {0, 0};

void initialize(void)
{

	printf("Iniitializing\n\n");

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, 1080,
							  1920, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	init_world(window);

	starting_spacing = .1f;
	spacing = starting_spacing;
	spacing_limit = 10;

	create_sprite(&background, "../assets/background.png", get_world()->screen_dimensions.y, get_world()->screen_dimensions.y, 0, 0);
	create_sprite(&dot_texture, "../assets/dot.png", 1000, 1000, 0, 0);

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init(renderer);
	init_box();
}

int main(void)
{
	initialize();
	// ADAPTIVE SYNC (-1) IMMEDIATE(0)
	SDL_GL_SetSwapInterval(1);

	Uint32 time_step_ms = 1000 / 60;
	Uint32 next_game_step = SDL_GetTicks();

	while (!quit)
	{
		Uint32 now = SDL_GetTicks();

		if(next_game_step <= now)
		{
			int computer_is_too_slow_limit = 30;

			while((next_game_step <= now) && (computer_is_too_slow_limit--))
			{
				update();
				draw();
				next_game_step += time_step_ms;
			}
			input();

			SDL_RenderPresent(renderer);

		} else
			SDL_Delay(next_game_step - now);
}
	cleanup();

	return 0;
}

void update(void)
{
	coordinate_location.x = origin_offset.x + get_world()->origin.x;
	coordinate_location.y = origin_offset.y + get_world()->origin.y;

	axes_vertical_line.rect.x = coordinate_location.x;
	axes_horizontal_line.rect.y = coordinate_location.y;
}


void draw_rect(SPRITE sprite)
{
	for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
	{
		ivec2 quadrant = {get_world()->origin.x, get_world()->origin.y};
		ivec2 distance = {0, 0};
		get_quadrant_pos(n, &quadrant, &distance);

		for(int i=0, y = quadrant.y; i < GRAPH_HEIGHT; i++, y+=distance.y)
		{
			for(int j =0, x = quadrant.x; j < GRAPH_WIDTH; j++, x+=distance.x)
			{
				// This part is actually updating(it's here to avoid using another loop)
				graph[n][i][j].x = x + origin_offset.x;
				graph[n][i][j].y = y + origin_offset.y;

				// This part is drawing
				SDL_RenderCopy(renderer, graph_box.texture, NULL, &graph[n][i][j]);
			}
		}
	}
}

void draw_points(void)
{
for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
	{
		ivec2 quadrant = {get_world()->origin.x, get_world()->origin.y};
		ivec2 distance = {0, 0};
		get_quadrant_pos(n, &quadrant, &distance);

		for(int i=0, y = quadrant.y; i < GRAPH_HEIGHT; i++, y+=distance.y)
		{
			for(int j =0, x = quadrant.x; j < GRAPH_WIDTH; j++, x+=distance.x)
			{
				// This part is actually updating(it's here to avoid using another loop)
				points[n][i][j].rect.x = x + origin_offset.x - points[n][i][j].rect.w/2;
				points[n][i][j].rect.y = y + origin_offset.y - points[n][i][j].rect.h/2;

				// This part is drawing
				if(points[n][i][j].is_visible)
					SDL_RenderCopy(renderer, dot_texture.texture, NULL, &points[n][i][j].rect);
			}
		}
	}
}

void draw(void)
{
	// set background color and clear screen with it
	SDL_SetRenderDrawColor(renderer, 17, 7, 12, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, background.texture, NULL, NULL);


	draw_rect(graph_box);
	// draw axes()
	SDL_RenderCopy(renderer, axes_horizontal_line.texture, NULL, &axes_horizontal_line.rect);
	SDL_RenderCopy(renderer, axes_vertical_line.texture, NULL, &axes_vertical_line.rect);
	draw_points();
	// draw numbers
	int number = 1;
	int starting_line = 1;
	draw_numbers(number, starting_line, D_RIGHT);
	number = -1;
	starting_line = -1;
	draw_numbers(number, starting_line, D_LEFT);
	number = 1;
	starting_line = 1;
	draw_numbers(number, starting_line, D_UP);
	number = -1;
	starting_line = -1;
	draw_numbers(number, starting_line, D_DOWN);
}

void draw_numbers(int number, int starting_line, enum DIRECTION direction)
{
	int font_size = MIN_FONT_SIZE;
	int line = starting_line;

	for(int i = MIN_FONT_SIZE; i < MAX_FONT_SIZE; i++)
	{
		SPRITE *d_font = load_texture(i);

		if((d_font->rect.w*2) == spacing)
		{
			font_size = i;
		}
	}
	SPRITE *d_font = load_texture(font_size);
	bool is_negative = direction == D_LEFT || direction == D_DOWN ? true : false;
	number = number > 0 ? number : -number;
	int part_size = d_font->rect.w / 10;

	d_font->rect.x = (get_world()->origin.x + origin_offset.x);
	d_font->rect.y = (get_world()->origin.y + origin_offset.y);

	SDL_Rect rect_place;
	// This selects a part from the texture
	// as always, we start counting at 0 - (part_size * number) gives us that number part
	rect_place.x = part_size*0;
	rect_place.y = 0;
	rect_place.w = part_size;
	rect_place.h = d_font->rect.h;

	// This is the actually coordinate location to be drawn at
	SDL_Rect dest;
	dest.x = (get_world()->origin.x + origin_offset.x);
	dest.y = (get_world()->origin.y + origin_offset.y);

	// draw zero
	SDL_Rect zero_part;
	zero_part.x = 0;
	zero_part.y = 0;
	zero_part.w = part_size;
	zero_part.h = d_font->rect.h;
	SDL_RenderCopy(renderer, d_font->texture, &zero_part, &dest);

	switch (direction) {
			case D_LEFT:
				dest.x += (starting_line * get_world()->world_dimensions.x);
				break;
			case D_RIGHT:
				dest.x += (starting_line * get_world()->world_dimensions.x);
				break;
			case D_DOWN:
				dest.y += -(starting_line * get_world()->world_dimensions.y);
				break;
			case D_UP:
				dest.y += -(starting_line * get_world()->world_dimensions.y);
				break;
				case D_NONE:
					break;
	}
	dest.w = part_size;
	dest.h = d_font->rect.h;
	//dest.x+= part_size;

	for(int i=1; i < GRAPH_WIDTH; i++)
	{
		int divisor = 10;
		int distance_left = get_number(i, 0);
		int distance = distance_left;
		int place = 0;
		int j=i;
		SDL_Rect dest_num = dest;

		dest.x += direction == D_LEFT || direction == D_DOWN ? (part_size*distance_left) : part_size*distance_left;

		do {
			place = j % divisor;
			rect_place.x = part_size * place;

			SDL_RenderCopy(renderer, d_font->texture, &rect_place, &dest);

			dest.x += -part_size;
			distance_left--;
			j = (j / divisor) ? j/divisor : 1;
		} while(distance_left > 0);
		if(is_negative)
		{
			SPRITE *s_font = load_sign(font_size);
			SDL_RenderCopy(renderer, s_font->texture, NULL, &dest);
		}
		switch (direction) {
			case D_LEFT:
				dest.x += (distance - get_world()->world_dimensions.x);
				break;
			case D_RIGHT:
				dest.x += (distance + get_world()->world_dimensions.x);
				break;
			case D_DOWN:
				dest.y += (distance + get_world()->world_dimensions.y);
				break;
			case D_UP:
				dest.y += (distance - get_world()->world_dimensions.y);
				break;
				case D_NONE:
					break;
		}
	}
}

int get_number(int num, int part_size)
{
	if(!num) return part_size;
	else {
		part_size += 1;
		return get_number(num/=10, part_size);
	}
}

bool mouse_moved = false;
bool mouse_held = false;
bool first_click = false;
int first_click_x = 0;
int first_click_y = 0;
Uint32 buttons;
bool left_mousedown = false;

void mouse_event()
{
	ivec2 mouse_units;
	ivec2 mouse_raw;
	if (left_mousedown)
	{
		SDL_GetMouseState(&mouse_raw.x, &mouse_raw.y);
		mouse_units.x = mouse_raw.x;
		mouse_units.y = mouse_raw.y;
		mouse_units = conv_units(mouse_raw.x, mouse_raw.y);

		if (first_click == false)
		{
			first_click = true;
			first_click_x = mouse_raw.x;
			first_click_y = mouse_raw.y;

			SDL_Point point = { mouse_raw.x, mouse_raw.y } ;

			for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
				{
					ivec2 quadrant = {get_world()->origin.x, get_world()->origin.y};
					ivec2 distance = {0, 0};

					get_quadrant_pos(n, &quadrant, &distance);

					for(int i=0, y = quadrant.y; i < GRAPH_HEIGHT; i++, y+=distance.y)
					{
						for(int j =0, x = quadrant.x; j < GRAPH_WIDTH; j++, x+=distance.x)
						{
							if(SDL_PointInRect(&point, &points[n][i][j].rect))
								points[n][i][j].is_visible = true;
						}
					}
				}
			}
		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse
		origin_offset.x += (((first_click_x - mouse_raw.x)) / mouse_speed.x);
		origin_offset.y += (((first_click_y - mouse_raw.y)) / mouse_speed.y);
	}
}

void input()
{
	quit = false;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				quit = true;
				cleanup();
				break;
			case SDL_MOUSEBUTTONDOWN:

				SDL_GetMouseState(&mouse_x, &mouse_y);
				mouse_moved = true;
				if ((e.type & SDL_BUTTON_LMASK) != 0)
				{
					left_mousedown = true;
				}
				if ((buttons & SDL_BUTTON_LMASK) != 0)
				{
				}
				break;
				case SDL_FINGERDOWN:
				SDL_GetMouseState(&mouse_x, &mouse_y);
				mouse_moved = true;
				left_mousedown = true;
				break;
			case SDL_MOUSEBUTTONUP:
				left_mousedown = false;
				mouse_moved = false;
				// if((e.type & SDL_BUTTON_LMASK) != 0)
				// {
				first_click_x = 0;
				first_click_y = 0;

				first_click = false;
				// }
				break;
			case SDL_FINGERUP:
				left_mousedown = false;
				mouse_moved = false;
				first_click_x = 0;
				first_click_y = 0;
				first_click = false;
				break;
			case SDL_MOUSEWHEEL:
				if (e.wheel.y > 0)
				{
					for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
					{

						int quadrant_x = get_world()->origin.x;
						int quadrant_y = get_world()->origin.y;

						for(int i=0; i < GRAPH_HEIGHT; i++)
							{
								for(int j =0; j < GRAPH_WIDTH; j++)
								{
									graph[n][i][j].x-=spacing;
									graph[n][i][j].y-=spacing;

									graph[n][i][j].w-=spacing;
									graph[n][i][j].h-=spacing;
								}
							}
					}
					if (spacing > spacing_limit)
						spacing = starting_spacing;
					else
					{
						spacing+=.1;
					}
				}
				else if (e.wheel.y < 0)
				{
					for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
					{

						int quadrant_x = get_world()->origin.x;
						int quadrant_y = get_world()->origin.y;

						for(int i=0; i < GRAPH_HEIGHT; i++)
						{
							for(int j =0; j < GRAPH_WIDTH; j++)
							{
								graph[n][i][j].w+=spacing;
								graph[n][i][j].h+=spacing;
							}
						}
					}
					if (spacing <= starting_spacing)
					{
						spacing = spacing_limit;
					}
					else
					{
						spacing-=.1;
					}
			}
			case SDL_WINDOWEVENT:
				update_world(e);
				// Change graphbox coordinates to new window size
				init_box();
				break;
			}
	}

	mouse_event();
	SDL_PumpEvents();
}

void init_box()
{
	starting_spacing = .1;
	spacing = starting_spacing;
	spacing_limit = 10;

	// Currently a bug. I think the conversion to world units is returning zero at a certain point in vertical orientation
	// and makes vertical and horizontal line disappear
	// starting x is -render_distance * 3 fills screen left, middle, and right
	if(get_world()->screen_dimensions.x > get_world()->screen_dimensions.y)
	{
		axes_horizontal_line_width = get_world()->render_distance.x * 3;
		axes_horizontal_line_height = 1*get_world()->world_dimensions.y;

		axes_vertical_line_width = 1*get_world()->world_dimensions.x;
		axes_vertical_line_height = get_world()->render_distance.y * 3;
	} else
	{
		axes_horizontal_line_width = get_world()->render_distance.x * 1;
		axes_horizontal_line_height = 3*get_world()->world_dimensions.y;

		axes_vertical_line_width = 3*get_world()->world_dimensions.x;
		axes_vertical_line_height = get_world()->render_distance.y * 1;
	}

	// Cleanup past textures
	if(axes_horizontal_line.texture)
		SDL_DestroyTexture(axes_horizontal_line.texture);
	if(axes_vertical_line.texture)
		SDL_DestroyTexture(axes_vertical_line.texture);
	if(graph_box.texture)
		SDL_DestroyTexture(graph_box.texture);

	create_sprite(&axes_horizontal_line, "../assets/horizontal_line.png",
					axes_horizontal_line_width, axes_horizontal_line_height, -get_world()->render_distance.x, get_world()->origin.y);
	create_sprite(&axes_vertical_line, "../assets/vertical_line.png",
					axes_vertical_line_width, axes_vertical_line_height, get_world()->origin.x, -get_world()->render_distance.y);

	create_sprite(&graph_box, "../assets/rect.png", 1*get_world()->world_dimensions.y, 1*get_world()->world_dimensions.y, 0, 0);

	for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
	{
		ivec2 quadrant = {get_world()->origin.x, get_world()->origin.y};
		ivec2 distance = {0, 0};

		get_quadrant_pos(n, &quadrant, &distance);

		for(int i=0, y = quadrant.y; i < GRAPH_HEIGHT; i++, y+=distance.y)
		{
			for(int j =0, x = quadrant.x; j < GRAPH_WIDTH; j++, x+=distance.x)
			{

				graph[n][i][j].x = x+origin_offset.x;
				graph[n][i][j].y = y+origin_offset.y;
				graph[n][i][j].w = 1*get_world()->world_dimensions.x;
				graph[n][i][j].h = 1*get_world()->world_dimensions.y;

				switch (n) {
					case 0:
						points[n][i][j].rect.w = 10;
						points[n][i][j].rect.h = 10;
						break;
					case 1:
						points[n][i][j].rect.w = 10;
						points[n][i][j].rect.h = 10;
						break;
					case 2:
						points[n][i][j].rect.w = 10;
						points[n][i][j].rect.h = 10;
						break;
					case 3:
						points[n][i][j].rect.w = 10;
						points[n][i][j].rect.h = 10;
						break;
				}
			}
		}
	}
}

static void create_sprite(SPRITE* sprite, char* path, int width, int height, int x, int y)
{
	SDL_Surface *surface = IMG_Load(path);
	sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
	sprite->rect.w = width;
	sprite->rect.h = height;
	sprite->rect.x = x;
	sprite->rect.y = y;
	SDL_FreeSurface(surface);
}

static void get_quadrant_pos(int location, ivec2 *quadrant, ivec2 *distance)
{
	switch (location)
	{
			case 0:
				quadrant->x += get_world()->world_dimensions.x/2;
				quadrant->y -= get_world()->world_dimensions.y/2;
				distance->x = get_world()->world_dimensions.x;
				distance->y = -get_world()->world_dimensions.y;
				break;
			case 1:
				quadrant->x -= get_world()->world_dimensions.x/2;
				quadrant->y -= get_world()->world_dimensions.y/2;
				distance->x = -get_world()->world_dimensions.x;
				distance->y = -get_world()->world_dimensions.y;
				break;
			case 2:
				quadrant->x -= get_world()->world_dimensions.x/2;
				quadrant->y += get_world()->world_dimensions.y/2;
				distance->x = -get_world()->world_dimensions.x;
				distance->y = get_world()->world_dimensions.y;
				break;
			case 3:
				quadrant->x += get_world()->world_dimensions.x/2;
				quadrant->y += get_world()->world_dimensions.y/2;
				distance->x = get_world()->world_dimensions.x;
				distance->y = get_world()->world_dimensions.y;
				break;
	}
}

void cleanup(void)
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(axes_vertical_line.texture);
	SDL_DestroyTexture(axes_horizontal_line.texture);
	font_cleanup();
	printf("CLEANUP\n");
	SDL_Quit();
}
