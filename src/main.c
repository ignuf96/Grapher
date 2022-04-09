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
#include <unistd.h>
#include <SDL2/SDL_platform.h>
// Font things
#include "../include/font_handler.h"
//
// Window information for initialization
#define WINDOW_TITLE "Grapher"
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED

// dimensions 16:9
static int pixel_width;
static int pixel_height;
#define PIXEL_SCALE 1

static int axes_horizontal_line_width, axes_horizontal_line_height;
static int axes_vertical_line_width, axes_vertical_line_height;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Event e;

static int window_width_raw;
static int window_height_raw;

typedef struct FLOAT_VECTOR2
{
	float x;
	float y;

}fvec2;

typedef struct INT_VECTOR2
{
	int x;
	int y;
}ivec2;

static ivec2 origin;
// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
static ivec2 origin_offset = { 0, 0 };
// Lower means faster
static ivec2 mouse_speed = { 60, 60 };
static ivec2 coordinate_location = { 0, 0 };
static ivec2 render_distance;

const static struct INT_VECTOR2 ASPECT_RATIO = { 16, 9};

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
static char str_buffer[MAX_STR_BUFFER-1];
static char str_num_buffer[MAX_STR_BUFFER];

static bool quit = false;

static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void update(void);
static void draw_coordinates(void);
static void draw_numbers(int position, int offset, enum DIRECTION direction);
static ivec2 conv_units(int a, int b);
static ivec2 conv_raw(int a, int b);
static void create_sprite(SPRITE* sprite, char* path, int width, int height, int x, int y);
static bool is_on_screen(SDL_Rect rect);
static void draw_mouse_coordinates(int x, int y);
static int get_number(int num, int part_size);
static void init_box(void);

SPRITE graph_box;

#define NUMBER_OF_QUADRANTS 4

// has to be same width and height or else causes bug
#define GRAPH_WIDTH 300
#define GRAPH_HEIGHT 300

SDL_Rect graph[NUMBER_OF_QUADRANTS][GRAPH_WIDTH][GRAPH_HEIGHT];

void initialize(void)
{

	printf("Iniitializing\n\n");

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, 800,
							  600, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_GetWindowSize(window, &window_width_raw, &window_height_raw);

	pixel_width = window_width_raw/ASPECT_RATIO.x;
	pixel_height = window_height_raw/ASPECT_RATIO.y;
	starting_spacing = .1f;
	spacing = starting_spacing;
	spacing_limit = 10;

	origin.x = window_width_raw/2;
	origin.y = window_height_raw/2;
	render_distance.x = window_width_raw;
	render_distance.y = window_height_raw;

	// starting x is -render_distance * 3 fills screen left, middle, and right
	axes_horizontal_line_width = render_distance.x * 3;
	axes_horizontal_line_height = 1*pixel_height;

	axes_vertical_line_width = 1*pixel_width;
	axes_vertical_line_height = render_distance.y * 3;

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init(renderer);
	init_box();
}

int main(void)
{
	//printf("Get number: %d\n", get_number(10, 0));
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
	coordinate_location.x = origin_offset.x + origin.x;
	coordinate_location.y = origin_offset.y + origin.y;

	for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
	{

		int quadrant_x = origin.x;
		int quadrant_y = origin.y;

		int distance_x = 0;
		int distance_y = 0;
		switch (n) {
			case 0:
				quadrant_x += pixel_width/2;
				quadrant_y -= pixel_height/2;
				distance_x = pixel_width;
				distance_y = -pixel_height;
				break;
			case 1:
				quadrant_x -= pixel_width/2;
				quadrant_y -= pixel_height/2;
				distance_x = -pixel_width;
				distance_y = -pixel_height;
				break;
			case 2:
				quadrant_x -= pixel_width/2;
				quadrant_y += pixel_height/2;
				distance_x = -pixel_width;
				distance_y = pixel_height;
				break;
			case 3:
				quadrant_x += pixel_width/2;
				quadrant_y += pixel_height/2;
				distance_x = pixel_width;
				distance_y = pixel_height;
				break;
		}
		for(int i=0, y = quadrant_y; i < GRAPH_HEIGHT; i++, y+=distance_y)
		{
			for(int j =0, x = quadrant_x; j < GRAPH_WIDTH; j++, x+=distance_x)
			{

				graph[n][i][j].x = x + origin_offset.x;
				graph[n][i][j].y = y + origin_offset.y;
			}
		}
	}
	axes_vertical_line.rect.x = coordinate_location.x;
	axes_horizontal_line.rect.y = coordinate_location.y;
}

void draw_rect(SPRITE sprite)
{
	for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
	{
		for(int i = 0; i < GRAPH_HEIGHT; i++)
		{
			for(int j = 0; j < GRAPH_WIDTH; j++)
			{
				SDL_RenderCopy(renderer, graph_box.texture, NULL, &(graph[n][i][j]));
			}
		}
	}
}

void draw(void)
{
	// set background color and clear screen with it
	SDL_SetRenderDrawColor(renderer, 16, 0, 2, 255);
	SDL_RenderClear(renderer);

	// draw numbers
	int render_max = 300;
	int lines_to_draw = INT_MAX-1;
	int number = 1;
	int starting_line = 1;
	draw_numbers(number, starting_line, D_RIGHT);


	draw_rect(graph_box);

	//draw_axes();
	SDL_RenderCopy(renderer, axes_horizontal_line.texture, NULL, &axes_horizontal_line.rect);
	SDL_RenderCopy(renderer, axes_vertical_line.texture, NULL, &axes_vertical_line.rect);
}

void draw_numbers(int number, int starting_line, enum DIRECTION direction)
{
	int font_size = MIN_FONT_SIZE;
	int line = starting_line;

	for(int i = MIN_FONT_SIZE; i < MAX_FONT_SIZE; i++)
	{
		SPRITE *d_font = load_texture(1, i);

		if((d_font->rect.w*2) == spacing)
		{
			//printf("Similar size!!!%d", spacing);
			font_size = i;
		}
	}

	SPRITE *d_font = load_texture(10, font_size);
	int part_size = d_font->rect.w / 10;


	d_font->rect.x = (origin.x + origin_offset.x);
	d_font->rect.y = (origin.y + origin_offset.y);


	SDL_Rect rect_place;
	// This selects a part from the texture
	// as always, we start counting at 0 - (part_size * number) gives us that number part
	rect_place.x = part_size*0;
	rect_place.y = 0;
	rect_place.w = part_size;
	rect_place.h = d_font->rect.h;

	// This is the actually coordinate location to be drawn at
	SDL_Rect dest;
	dest.x = (origin.x + origin_offset.x);
	dest.y = (origin.y + origin_offset.y);
	dest.w = part_size;
	dest.h = d_font->rect.h;

	//SDL_RenderCopy(renderer, d_font->texture, &rect_place, &dest);


	int divisor = 1000;
	//int distance = 0;

	for(int i=1; i < 30; i++)
	{
		int distance_left = get_number(i, 0);
		int distance = distance_left;
		int place = 0;
		int j=i;
		SDL_Rect dest_num = dest;

		dest.x += part_size*distance_left;

		switch (distance_left) {
			case 1:
				divisor = 10;
				break;
			case 2:
				divisor = 10;
				break;
			case 3:
				divisor = 100;
				break;
		}

		do {
			//printf("I: %d\nDistance: %d\n", i, distance);

			//dest.y = (origin.y + origin_offset.y);
			place = j % divisor;
			rect_place.x = part_size * place;

			//printf("Running Num: %d\nJ: %d\nPlace: %d\nRect_place.x: %d\nDest.x: %d\n", i, j, place, rect_place.x, dest.x);

			//if(i > 19)
				//sleep(2);
			SDL_RenderCopy(renderer, d_font->texture, &rect_place, &dest);

			dest.x -= part_size;
			distance_left--;
			j = (j / divisor) ? 1 : j / divisor;
		} while(distance_left > 0);
		dest.x += (distance*pixel_width);

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

			printf("Clicked at\nx: %d\ny: %d\n", ((((mouse_raw.x-origin.x+origin_offset.x))/pixel_width)),
			   -(((mouse_raw.y)/pixel_height)-((origin_offset.y+origin.y+axes_horizontal_line_height)/pixel_height)));

		//	int temp_x = origin.x+origin_offset.x;
			//int temp_y = origin.y+origin_offset.y;
			//conv_ivec(&temp_x, &temp_y);
			//printf("Clicked at\nx: %d\ny: %d\n", (x)-temp_x,
				//   ((-y)-(((origin.y+origin_offset.y))/pixel_height)));

		}
		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse
		origin_offset.x += (((first_click_x - mouse_raw.x)) / mouse_speed.x);
		origin_offset.y += (((first_click_y - mouse_raw.y)) / mouse_speed.y);

		//printf("Moving X by: %d\nMoving Y by: %d\n", origin_offset.x, origin_offset.y);
	}
}
struct FLOAT_VECTOR2 gesture;

void input()
{
	quit = false;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_DOLLARGESTURE:

				break;
			case SDL_QUIT:
				quit = true;
				cleanup();
				break;
			case SDL_KEYDOWN:

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

						int quadrant_x = origin.x;
						int quadrant_y = origin.y;

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

						int quadrant_x = origin.x;
						int quadrant_y = origin.y;

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

		switch (e.window.event)
		{
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			// Change line coordinates to new window size
			window_width_raw = e.window.data1;
			window_height_raw = e.window.data2;

			printf("Height: %d\nWidth: %d\n", e.window.data2, e.window.data1);

			pixel_width = window_width_raw/ASPECT_RATIO.x;
			pixel_height = window_height_raw/ASPECT_RATIO.y;

			origin.x = window_width_raw/2;
			origin.y = window_height_raw/2;
			render_distance.x = window_width_raw;
			render_distance.y = window_height_raw;

			init_box();
		}
		break;
		}
	}

	mouse_event();
	SDL_PumpEvents();
}

void init_box()
{
	pixel_width = window_width_raw/ASPECT_RATIO.x;
	pixel_height = window_height_raw/ASPECT_RATIO.y;
	starting_spacing = .1;
	spacing = starting_spacing;
	spacing_limit = 10;

	origin.x = window_width_raw/2;
	origin.y = window_height_raw/2;
	render_distance.x = window_width_raw;
	render_distance.y = window_height_raw;

	// starting x is -render_distance * 3 fills screen left, middle, and right
	axes_horizontal_line_width = render_distance.x * 3;
	axes_horizontal_line_height = 1*pixel_height;

	axes_vertical_line_width = 1*pixel_width;
	axes_vertical_line_height = render_distance.y * 3;

	// Cleanup past textures
	if(axes_horizontal_line.texture)
		SDL_DestroyTexture(axes_horizontal_line.texture);
	if(axes_vertical_line.texture)
		SDL_DestroyTexture(axes_vertical_line.texture);
	if(graph_box.texture)
		SDL_DestroyTexture(graph_box.texture);

	create_sprite(&axes_horizontal_line, "../assets/horizontal_line.png",
					axes_horizontal_line_width, axes_horizontal_line_height, -render_distance.x, origin.y);
	create_sprite(&axes_vertical_line, "../assets/vertical_line.png",
					axes_vertical_line_width, axes_vertical_line_height, origin.x, -render_distance.y);

	create_sprite(&graph_box, "../assets/rect.png", 1*pixel_width, 1*pixel_height, 0, 0);

	for(int n=0; n < NUMBER_OF_QUADRANTS; n++)
	{
		int quadrant_x = origin.x;
		int quadrant_y = origin.y;

		int distance_x = 0;
		int distance_y = 0;

		switch (n) {
			case 0:
				quadrant_x += pixel_width/2;
				quadrant_y -= pixel_height/2;
				distance_x = pixel_width;
				distance_y = -pixel_height;
				break;
			case 1:
				quadrant_x -= pixel_width/2;
				quadrant_y -= pixel_height/2;
				distance_x = -pixel_width;
				distance_y = -pixel_height;
				break;
			case 2:
				quadrant_x -= pixel_width/2;
				quadrant_y += pixel_height/2;
				distance_x = -pixel_width;
				distance_y = pixel_height;
				break;
			case 3:
				quadrant_x += pixel_height/2;
				quadrant_y += pixel_height/2;
				distance_x = pixel_width;
				distance_y = pixel_height;
				break;
		}
		for(int i=0, y = quadrant_y; i < GRAPH_HEIGHT; i++, y+=distance_y)
		{
			for(int j =0, x = quadrant_x; j < GRAPH_WIDTH; j++, x+=distance_x)
			{

				graph[n][i][j].x = x+origin_offset.x;
				graph[n][i][j].y = y+origin_offset.y;
				graph[n][i][j].w = 1*pixel_width;
				graph[n][i][j].h = 1*pixel_height;
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

ivec2 conv_units(int x, int y)
{
	struct INT_VECTOR2 temp;

	temp.x /= pixel_width;
	temp.y /= pixel_height;

	return temp;
}

ivec2 conv_raw(int x, int y)
{
	struct INT_VECTOR2 temp;

	temp.x *= (window_width_raw/pixel_width);
	temp.y *= (window_height_raw/pixel_height);

	return temp;
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
