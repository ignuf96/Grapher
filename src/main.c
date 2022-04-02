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
#include "../include/platform.h"

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
static int graph_horizontal_line_width, graph_horizontal_line_height;
static int graph_vertical_line_width, graph_vertical_line_height;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Event e;

static int window_width_raw;
static int window_height_raw;

struct FLOAT_VECTOR2
{
	float x;
	float y;
};
struct INT_VECTOR2
{
	int x;
	int y;
};

static struct INT_VECTOR2 origin;
// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
static struct INT_VECTOR2 origin_offset = { 0, 0 };
// Lower means faster
static struct FLOAT_VECTOR2 mouse_speed = { 60, 60 };
static struct INT_VECTOR2 coordinate_location = { 0, 0 };
static struct INT_VECTOR2 render_distance;

const static struct INT_VECTOR2 ASPECT_RATIO = { 16, 9};

// Amount of space away from y and x axis
// Used for drawing lines in the quadrant
int spacing;
int starting_spacing;
int spacing_limit;

static float fps;
static const float FPS_TARGET = 60.0f;

static SPRITE axes_horizontal_line;
static SPRITE axes_vertical_line;
static SPRITE graph_horizontal_line;
static SPRITE graph_vertical_line;

static enum DIRECTION{D_UP, D_DOWN, D_LEFT, D_RIGHT, D_NONE}direction=D_NONE;

static int mouse_x, mouse_y;

#define MAX_STR_BUFFER 100
static char str_buffer[MAX_STR_BUFFER-1];
static char str_num_buffer[MAX_STR_BUFFER];

static bool quit = false;

//struct INT_VECTOR2 world_units = {16,9};

static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void draw_coordinates(void);
static void draw_numbers(int position, int offset, enum DIRECTION direction);
static void conv_ivec(int *a, int *b);
static void conv_fvec(float *a, float *b);
static void create_sprite(SPRITE* sprite, char* path, int width, int height, int x, int y);
static void draw_lines(int x, int y, int x1, int y2, int amount, SPRITE* sprite);
static bool is_on_screen(SDL_Rect rect);
static void draw_mouse_coordinates(int x, int y);
static char* strincat(char* str1, int num_str, unsigned long length);

static enum ORIENTATION {LANDSCAPE, PORTRAIT} orientation;

SPRITE graph_rect;

void initialize(void)
{

	printf("Iniitializing\n\n");

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, 1920,
							  1080, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_GetWindowSize(window, &window_width_raw, &window_height_raw);

	if(window_width_raw > window_height_raw)
	{
		orientation = LANDSCAPE;
	}
	else if(window_height_raw > window_width_raw)
	{
		orientation = PORTRAIT;
	}

	switch(orientation)
	{
		case LANDSCAPE:
			pixel_width = window_width_raw/ASPECT_RATIO.x;
			pixel_height = window_height_raw/ASPECT_RATIO.y;

			break;
		case PORTRAIT:
			pixel_width = window_width_raw/ASPECT_RATIO.x;
			pixel_height = window_height_raw/ASPECT_RATIO.y;

			break;
	}
	starting_spacing = 1 * pixel_width;
	spacing = starting_spacing;
	spacing_limit = 3 * pixel_width;


	origin.x = window_width_raw/2;
	origin.y = window_height_raw/2;
	render_distance.x = window_width_raw;
	render_distance.y = window_height_raw;

	axes_horizontal_line_width = render_distance.x;
	axes_horizontal_line_height = 1*pixel_height;

	axes_vertical_line_width = 1*pixel_width;
	axes_vertical_line_height = render_distance.y;

	graph_horizontal_line_width = render_distance.x;
	graph_horizontal_line_height = 1*pixel_height;

	graph_vertical_line_height = render_distance.y;
	graph_vertical_line_width = 1*pixel_width;

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init(renderer);

	create_sprite(&axes_horizontal_line, "../assets/horizontal_line.png",
					axes_horizontal_line_width, axes_horizontal_line_height, 0, pixel_height/2);
	create_sprite(&graph_horizontal_line, "../assets/graph_horizontal_line.png",
					graph_horizontal_line_width, graph_horizontal_line_height, 0, pixel_height/2);
	create_sprite(&axes_vertical_line, "../assets/vertical_line.png",
					axes_vertical_line_width, axes_vertical_line_height, pixel_width/2, 0);
	create_sprite(&graph_vertical_line, "../assets/graph_vertical_line.png",
					graph_vertical_line_width, graph_vertical_line_height, pixel_width/2, 0);

	create_sprite(&graph_rect, "../assets/rect.png", 1*pixel_width, 1*pixel_height, 0, 0);

	conv_fvec(&mouse_speed.x, &mouse_speed.y);
}

int main(void)
{
	initialize();
	// ADAPTIVE SYNC (-1) IMMEDIATE(0)
	SDL_GL_SetSwapInterval(1);

	Uint32 time_step_ms = 1000 / 60;
	Uint32 next_game_step = SDL_GetTicks(); // initial value
											//
	while (!quit)
	{
		Uint32 now = SDL_GetTicks();

		if(next_game_step <= now)
		{
			int computer_is_too_slow_limit = 30;


			while((next_game_step <= now) && (computer_is_too_slow_limit--))
			{
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

#define MAX_DRAW_RECT 1

void draw_rect(SPRITE sprite)
{

	SDL_Rect rect_pos = sprite.rect;
		rect_pos.w += spacing;
		rect_pos.h += spacing;
		for(int y=-MAX_DRAW_RECT, distancey=pixel_height; y < MAX_DRAW_RECT; y++, distancey+=pixel_height)
{
		rect_pos.y = y+distancey;

		for(int x=-MAX_DRAW_RECT, distancex=pixel_width; x < MAX_DRAW_RECT; x++, distancex+=pixel_width)
		{
			rect_pos.x = x+distancex;
			SDL_RenderCopy(renderer, sprite.texture, NULL, &rect_pos);
		}
	}
}

void draw(void)
{
	SDL_SetRenderDrawColor(renderer, 16, 37, 73, 255);
	SDL_RenderClear(renderer);

	coordinate_location.x = origin_offset.x + origin.x;
	coordinate_location.y = origin_offset.y + origin.y;
	//draw_axes();
	int render_max = 300;
	int lines_to_draw = INT_MAX-1;

	int number = 1;
	int starting_line = 1;
	draw_numbers(number, starting_line, D_RIGHT);
	draw_rect(graph_rect);
}

/*
bool is_on_screen(SDL_Rect rect)
{
	bool is_in_view = false;

	if((rect.x) > (0 + origin_offset.x) && (rect.x+origin_offset.x) < (pixel_width + origin_offset.x))
	{
		is_in_view = true;
	}
	return is_in_view;
}
*/

float fconv_raw(float a)
{
	a *= ((float)window_width_raw / (pixel_width));

	return a;
}

int iconv_raw(int a)
{

	a *= (window_width_raw / (pixel_width));

	return a;
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

	SPRITE *d_font = load_texture(1, font_size);
	int part_size = d_font->rect.w / 10;

	int divisor = 1000;
	{

		for(int i=1; i < 300; i++)
		{
			int place = 0;
			int distance = 0;
			int j=i;
		do {
			SDL_Rect rect_place = d_font->rect;
			d_font->rect.x = (origin.x + origin_offset.x + distance);
			d_font->rect.y = (origin.y + origin_offset.y);
			rect_place.x += place;
			SDL_RenderCopy(renderer, d_font->texture, NULL, &rect_place);

			j/=10;
			place+=part_size;
			distance+=spacing;

			} while((j / 10) != 0);
		}
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
	int x, y, raw_x, raw_y;
	if (left_mousedown)
	{
		SDL_GetMouseState(&raw_x, &raw_y);
		x = raw_x;
		y = raw_y;
		conv_ivec(&x, &y);

		if (first_click == false)
		{
			first_click = true;
			first_click_x = x;
			first_click_y = y;

			printf("Clicked at\nx: %d\ny: %d\n", ((((raw_x-origin.x+origin_offset.x))/pixel_width)),
			   -(((raw_y)/pixel_height)-((origin_offset.y+origin.y+axes_horizontal_line_height)/pixel_height)));

		//	int temp_x = origin.x+origin_offset.x;
			//int temp_y = origin.y+origin_offset.y;
			//conv_ivec(&temp_x, &temp_y);
			//printf("Clicked at\nx: %d\ny: %d\n", (x)-temp_x,
				//   ((-y)-(((origin.y+origin_offset.y))/pixel_height)));

		}
		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse
		origin_offset.x += (((first_click_x - x)) / mouse_speed.x);
		origin_offset.y += (((first_click_y - y)) / mouse_speed.y);

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
			case SDL_MULTIGESTURE:
				gesture.x = e.mgesture.x;
				gesture.y = e.mgesture.y;
				float rotationf = e.mgesture.dTheta;
				float pinched = e.mgesture.dDist;
				int num_fingers = e.mgesture.numFingers;

				printf("Gesture x: %f\nGesture y: %f\n", gesture.x, gesture.y);
				printf("Rotation: %f\nAmount Pinched: %f\n", rotationf, pinched);
				printf("Number of fingers: %d\n", num_fingers);

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
					if (spacing > spacing_limit)
						spacing = starting_spacing;
					else
						spacing+=3;
				}
				else if (e.wheel.y < 0)
				{
					if (spacing <= starting_spacing)
						spacing = spacing_limit;
					else
						spacing-=3;
				}
	case SDL_WINDOWEVENT:

				switch (e.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					/*
					** Change line coordinates to new window size
					*/
					// printf("First time called\n");
					// sleep(5);
					window_width_raw = e.window.data1;
					window_height_raw = e.window.data2;

					printf("Height: %d\nWidth: %d\n", e.window.data2, e.window.data1);

					if(window_width_raw > window_height_raw)
					{
						orientation = LANDSCAPE;
					}
					else if(window_height_raw > window_width_raw)
					{
						orientation = PORTRAIT;
					}

					pixel_width = window_width_raw/ASPECT_RATIO.x;
					pixel_height = window_height_raw/ASPECT_RATIO.y;


					origin.x = window_width_raw/2;
					origin.y = window_height_raw/2;
					render_distance.x = window_width_raw;
					render_distance.y = window_height_raw;

					break;
				}
			}
	}

	mouse_event();
	SDL_PumpEvents();
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

void conv_ivec(int *x, int *y)
{
	*x /= pixel_width;
	*y /= pixel_height;
}

void conv_fvec(float *x, float *y)
{
	*x /= pixel_width;
	*y /= pixel_height;
}

void cleanup(void)
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(axes_vertical_line.texture);
	SDL_DestroyTexture(axes_horizontal_line.texture);
	SDL_DestroyTexture(graph_vertical_line.texture);
	SDL_DestroyTexture(graph_horizontal_line.texture);
	font_cleanup();
	printf("CLEANUP\n");
	SDL_Quit();
}
