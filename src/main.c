/* TODO:
   1. Implement a constant frame-rate loop. Possibly 30-60 fps. This will help with smoothness of mouse gestures.
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_touch.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <limits.h>				/* MAX VALUE FOR GRAPH LONG_MAX */
#include <unistd.h>				// For sleep();
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
const static int pixel_width = 16*4;
const static int pixel_height = 9*4;
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

const static struct INT_VECTOR2 origin = { pixel_width/2, pixel_height/2};
// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
static struct INT_VECTOR2 origin_offset = { 0, 0 };
static struct FLOAT_VECTOR2 mouse_speed = { 200, 200 };
static struct INT_VECTOR2 coordinate_location = { 0, 0 };
const static struct INT_VECTOR2 render_distance = {pixel_width, pixel_height};

// Amount of space away from y and x axis
// Used for drawing lines in the quadrant
#define STARTING_SPACING 2.0
static int spacing = STARTING_SPACING;
#define SPACING_LIMIT 8.0

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

struct INT_VECTOR2 world_units = {16,9};

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

void initialize(void)
{

	printf("Iniitializing\n\n");

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, 1920,
							  1080, SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_SetWindowResizable(window, false);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_GetWindowSize(window, &window_width_raw, &window_height_raw);

	axes_horizontal_line_width = render_distance.x;
	axes_horizontal_line_height = 1*PIXEL_SCALE;

	axes_vertical_line_width = 1*PIXEL_SCALE;
	axes_vertical_line_height = render_distance.y;

	graph_horizontal_line_width = render_distance.x;
	graph_horizontal_line_height = 1*PIXEL_SCALE;

	graph_vertical_line_height = render_distance.y;
	graph_vertical_line_width = 1*PIXEL_SCALE;

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

void draw(void)
{
	SDL_SetRenderDrawColor(renderer, 16, 37, 73, 255);
	SDL_RenderClear(renderer);

	// axes in imaginary size so it's scaled up or down depending on actual
	// dimensions
	SDL_RenderSetLogicalSize(renderer, pixel_width, pixel_height);

	coordinate_location.x = origin_offset.x + origin.x;
	coordinate_location.y = origin_offset.y + origin.y;
	//draw_axes();
	int render_max = 300;
	int lines_to_draw = INT_MAX-1;
	// setting coordinate_location.x or coordinate_location.y makes the lines move as the mouse moves
	// Right now, the lines are limited by pixel_width or pixel_height.
	// We want to draw the minimum as to not impact performance. This will have the consequence that we will have to make an illusion
	// so it appears there's an infinite graph
	draw_lines(coordinate_location.x, -pixel_height,
			   coordinate_location.x, pixel_height, render_max, &graph_vertical_line);
	draw_lines(-pixel_width+(origin_offset.x), 0+coordinate_location.y, pixel_width+(origin_offset.x),
			   0+coordinate_location.y, render_max, &graph_horizontal_line);


	draw_lines(coordinate_location.x, -pixel_height+(origin_offset.y), coordinate_location.x,
			   pixel_height+(origin_offset.y), -render_max, &graph_vertical_line);
	draw_lines(-pixel_width+(origin_offset.x), 0+coordinate_location.y,
			   pixel_width+(origin_offset.x), 0+coordinate_location.y, -render_max, &graph_horizontal_line);
	draw_lines(coordinate_location.x, -pixel_height, coordinate_location.x, pixel_height, 1, &axes_vertical_line);
	draw_lines(-pixel_width, 0+coordinate_location.y, pixel_width, 0+coordinate_location.y, 1, &axes_horizontal_line);

	// using actual dimensions for text
	SDL_RenderSetLogicalSize(renderer, window_width_raw, window_height_raw);
	draw_coordinates();
	draw_mouse_coordinates(mouse_x, mouse_y);

	int number = 1;
	int starting_line = 1;
	draw_numbers(number, starting_line, D_RIGHT);

	number = -1;
	starting_line = -1;
	draw_numbers(number, starting_line, D_LEFT);

	number = 1;
	starting_line = 4;
	draw_numbers(number, starting_line, D_UP);

	number = -1;
	starting_line = 0;
	draw_numbers(number, starting_line, D_DOWN);

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

void draw_lines(int x, int y, int x1, int y2, int amount, SPRITE* sprite)
{
// This function will either draw one or more lines
// If x and x1 are the same value & amount > 1 then we assume only y is increasing and vice versa
// If amount is negative we get the inverse and draw the opposite way

	int draw_location;
	if(x == x1)
		draw_location = x;
	else if(y == y2)
		draw_location = y;

	int line_distance;

	if(amount >= 0)
		line_distance = spacing;
	else if(amount < 0)
	{
		line_distance = -spacing;
		amount = -amount;
	}

	for (int count = 1, axis = draw_location; count <= amount;
		axis += (line_distance), count++)
	{
		if(x == x1)
			sprite->rect.x = axis;
		else if(y == y2){
			sprite->rect.y = axis;
		}
//		if(is_on_screen(sprite->rect))
	//	{
			SDL_RenderCopy(renderer, sprite->texture, NULL,
						   &(sprite->rect));
		//}
	}
}

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

	switch(spacing)
	{
		case 2:
			font_size = MIN_FONT_SIZE;
			break;
		case 3:
			font_size = MIN_FONT_SIZE+1;
			break;
		case 4:
			font_size = MIN_FONT_SIZE+2;
			break;
		case 5:
			font_size = MIN_FONT_SIZE+3;
			break;
		case 6:
			font_size = MIN_FONT_SIZE+4;
			break;
		case 7:
			font_size = MIN_FONT_SIZE+5;
			break;
		case 8:
			font_size = MIN_FONT_SIZE+6;
			break;
		default:
			font_size = MIN_FONT_SIZE;
	}

	enum NUMBER_SIGN{NEGATIVE, POSITIVE}number_sign;
	if(number < 0)
	{
		number_sign = NEGATIVE;
		number = -(number);
	}
	else
	{
		number_sign = POSITIVE;
	}

	int distance = spacing;
	for (; number < MAX_FONT_NUMBERS; number++, line += ((direction==D_UP || direction==D_RIGHT) ? distance : -distance))
	{

		int num = number_sign ? number : -number;
		SPRITE *d_font = load_number(num, font_size);


		if(direction == D_LEFT)
		{
			d_font->rect.x = iconv_raw(origin.x + origin_offset.x + (line+distance));
			d_font->rect.y = iconv_raw(origin.y + origin_offset.y);
		}else if(direction == D_RIGHT)
		{
			d_font->rect.x = (origin.x + origin_offset.x + (line+distance)) * ((window_width_raw / (pixel_width))) + (d_font->rect.w);
			d_font->rect.y = (origin.y + origin_offset.y) * ((window_height_raw / (pixel_height)));
		} else if(direction == D_DOWN){
			d_font->rect.x = (origin.x + origin_offset.x) * (window_width_raw / (pixel_width)) + (d_font->rect.w+60);
			d_font->rect.y = (origin.y + origin_offset.y + (-line)+distance) * ((window_height_raw / (pixel_height))) + (d_font->rect.h)+20;
		} else if(direction == D_UP)
		{
			d_font->rect.x = (origin.x + origin_offset.x) * (window_width_raw / (pixel_width)) + (d_font->rect.w+60);
			d_font->rect.y = (origin.y + origin_offset.y + (-line)+distance) * ((window_height_raw / (pixel_height))) + (-d_font->rect.h)+20;
		}
		SDL_RenderCopy(renderer, d_font->texture, NULL,
					&(d_font->rect));
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
	int x, y;
	if (left_mousedown)
	{
		SDL_GetMouseState(&x, &y);
		conv_ivec(&x, &y);

		if (first_click == false)
		{
			first_click = true;
			first_click_x = x;
			first_click_y = y;
		}
		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse
		origin_offset.x += (((first_click_x - x)) / mouse_speed.x);
		origin_offset.y += (((first_click_y - y)) / mouse_speed.y);

		//printf("Moving X by: %d\nMoving Y by: %d\n", origin_offset.x, origin_offset.y);
	}
}


void draw_mouse_coordinates(int x, int y)
{
	SDL_Texture *texture_x = NULL;
	SPRITE temp_font_x;
	char *finalized_str_x = strincat("Mouse X: ", ((mouse_x/pixel_width)+(origin.x/pixel_width)), MAX_STR_BUFFER-1);

	load_string_font(renderer, &temp_font_x, texture_x, finalized_str_x);
	temp_font_x.rect.y = 175;
	SDL_RenderCopy(renderer, temp_font_x.texture, NULL, &temp_font_x.rect);
	SDL_DestroyTexture(temp_font_x.texture);

	SDL_Texture *texture_y = NULL;
	SPRITE temp_font_y;
	char *finalized_str_y = strincat("MOUSE Y: ", (mouse_y/pixel_height), MAX_STR_BUFFER-1);

	load_string_font(renderer, &temp_font_y, texture_y, finalized_str_y);
	temp_font_y.rect.y = 225;
	SDL_RenderCopy(renderer, temp_font_y.texture, NULL, &temp_font_y.rect);

	SDL_DestroyTexture(temp_font_y.texture);
}

void draw_coordinates(void)
{
	// Draw X Coordinate
	SDL_Texture *texture_x = NULL;
	SPRITE temp_font_x;
	char *finalized_str_x = strincat("Coordinate X: ", (int)-origin_offset.x, MAX_STR_BUFFER-1);

	load_string_font(renderer, &temp_font_x, texture_x, finalized_str_x);
	SDL_RenderCopy(renderer, temp_font_x.texture, NULL, &temp_font_x.rect);
	SDL_DestroyTexture(temp_font_x.texture);

	// Draw Y Coordinate
	SDL_Texture *texture_y = NULL;
	SPRITE temp_font_y;
	char *finalized_str_y = strincat("Coordinate Y: ", (int)origin_offset.y, MAX_STR_BUFFER-1);

	load_string_font(renderer, &temp_font_y, texture_y, finalized_str_y);
	temp_font_y.rect.y = 75;
	SDL_RenderCopy(renderer, temp_font_y.texture, NULL, &temp_font_y.rect);

	SDL_DestroyTexture(temp_font_y.texture);
}

// combine a string and a number into one string
char* strincat(char* str1, int num_str, unsigned long length)
{

	memset(&str_buffer, '\0', MAX_STR_BUFFER-1);
	memset(&str_num_buffer, '\0', MAX_STR_BUFFER-1);

	strncpy(str_buffer, str1, MAX_STR_BUFFER-1);
	snprintf(str_num_buffer, MAX_STR_BUFFER-1, "%d", num_str);

	char *finalized_str = strncat(str_buffer, str_num_buffer, MAX_STR_BUFFER-1);

	return finalized_str;
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
					if (spacing > SPACING_LIMIT)
						spacing = STARTING_SPACING;
					else
						spacing++;
				}
				else if (e.wheel.y < 0)
				{
					if (spacing <= STARTING_SPACING)
						spacing = SPACING_LIMIT;
					else
						spacing--;
				}

				break;
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
