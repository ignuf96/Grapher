/* TODO:
   1. Implement a constant frame-rate loop. Possibly 30-60 fps. This will help with smoothness of mouse gestures.
*/
#include <SDL2/SDL.h>
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

// Window information for initialization
#define WINDOW_TITLE "Grapher"
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED


// Let's try out same width and height. This will be our 'imaginary
// dimensions' 16:9 if landscape, 9:16 if portrait. If width > height we assume screen is landscape and vice versa.
int PIXEL_WIDTH;
int PIXEL_HEIGHT;

enum ORIENTATION {LANDSCAPE, PORTRAIT} orientation;

int horizontal_line_width, horizontal_line_height;
int vertical_line_width, vertical_line_height;
int graph_horizontal_line_width, graph_horizontal_line_height;
int graph_vertical_line_width, graph_vertical_line_height;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event e;

int window_width_raw;
int window_height_raw;

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

struct INT_VECTOR2 origin;
struct INT_VECTOR2 origin_offset = { 0, 0 };
struct FLOAT_VECTOR2 mouse_speed = { 211, 152 };
struct INT_VECTOR2 coordinate_location = { 0, 0 };
struct INT_VECTOR2 render_distance;

// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
// Amount of space away from y and x axis
// Used for drawing lines in the quadrants
static float spacing = 2.0f;
#define SPACING_LIMIT 22.0f

char *coordinate_str_x = "Coordinate X: ";
char *coordinate_str_y = "Coordinate Y: ";

char coordinate_x[MAX_FONT_COORDINATES];
char coordinate_y[MAX_FONT_COORDINATES];

float fps;
const float FPS_TARGET = 60.0f;

SPRITE horizontal_line;
SPRITE vertical_line;
SPRITE graph_horizontal_line;
SPRITE graph_vertical_line;

bool quit = false;

static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void draw_coordinates(void);
static void draw_numbers(int count, int distance_x, int distance_y, int spacing);
static void conv_ivec(int *a, int *b);
static void conv_fvec(float *a, float *b);
static void create_sprite(SPRITE* sprite, char* path, int width, int height, int x, int y);
static void draw_line(int x, int y, int render_distance_x, int render_distance_y, int spacing, SPRITE* sprite);

void initialize(void)
{

	printf("Iniitializing\n\n");

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, 0,
							  0, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_GetWindowSize(window, &window_width_raw, &window_height_raw);

	if(window_width_raw > window_height_raw)
	{
		orientation = LANDSCAPE;
		PIXEL_WIDTH = 16 * 6;
		PIXEL_HEIGHT = 9 * 6;
	} else
	{
		orientation = PORTRAIT;
		PIXEL_WIDTH = 9 * 6;
		PIXEL_HEIGHT = 16 * 6;
	}

	origin.x = PIXEL_WIDTH / 2;
	origin.y = PIXEL_HEIGHT / 2;

	render_distance.x = PIXEL_WIDTH * 10;
	render_distance.y = PIXEL_HEIGHT * 10;

	// Bug... Doesn't fill space with PIXEL_WIDTH
	horizontal_line_width = render_distance.x;
	horizontal_line_height = PIXEL_HEIGHT/13;
	vertical_line_width = PIXEL_WIDTH/13;
	vertical_line_height = render_distance.y;
	graph_horizontal_line_width = render_distance.x;
	graph_horizontal_line_height = PIXEL_HEIGHT/12;
	graph_vertical_line_height = render_distance.y;
	graph_vertical_line_width = PIXEL_WIDTH/16;

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init(renderer);

	create_sprite(&horizontal_line, "../assets/horizontal_line.png",
					horizontal_line_width, horizontal_line_height, 0, PIXEL_HEIGHT/2);
	create_sprite(&graph_horizontal_line, "../assets/graph_horizontal_line.png",
					graph_horizontal_line_width, graph_horizontal_line_height, 0, PIXEL_HEIGHT/2);
	create_sprite(&vertical_line, "../assets/vertical_line.png",
					vertical_line_width, vertical_line_height, PIXEL_WIDTH/2, 0);
	create_sprite(&graph_vertical_line, "../assets/graph_vertical_line.png",
					graph_vertical_line_width, graph_vertical_line_height, PIXEL_WIDTH/2, 0);

	conv_fvec(&mouse_speed.x, &mouse_speed.y);
}

int main(void)
{
	initialize();
	// ADAPTIVE SYNC (-1) IMMEDIATE(0)
	SDL_GL_SetSwapInterval(1);

	while (!quit)
	{
		Uint32 start_time = SDL_GetTicks();
		Uint32 frame_time;

		input();
		draw();

		frame_time = SDL_GetTicks() - start_time;
		fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;

		// Draw fps
		char buffconv[MAX_FONT_COORDINATES] = { 0 };
		strncat(buffconv, "FPS: ", MAX_FONT_COORDINATES - 1);
		snprintf(&buffconv[5], MAX_FONT_COORDINATES - 5, "%d", (int)fps);
		SDL_Texture *texture = NULL;
		struct SPRITE temp;
		load_string_font(renderer, &temp, texture, buffconv);
		temp.rect.x = 0;
		temp.rect.y = 200;

		SDL_RenderCopy(renderer, temp.texture, NULL, &temp.rect);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(temp.texture);
	}

	cleanup();

	return 0;
}

void draw(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// axes in imaginary size so it's scaled up or down depending on actual
	// dimensions
	SDL_RenderSetLogicalSize(renderer, PIXEL_WIDTH, PIXEL_HEIGHT);

	coordinate_location.x = origin_offset.x + origin.x;
	coordinate_location.y = origin_offset.y + origin.y;
	//draw_axes();
	draw_line(0, coordinate_location.y, 0, 1, 0, &horizontal_line);
	draw_line(coordinate_location.x, 0, 1, 0, 0, &vertical_line);

	draw_line(coordinate_location.x, 0, render_distance.x, 0, spacing, &graph_vertical_line);
	draw_line(coordinate_location.x, 0, render_distance.x, 0, -spacing, &graph_vertical_line);
	draw_line(0, coordinate_location.y, 0, render_distance.y, spacing, &graph_horizontal_line);
	draw_line(0, coordinate_location.y, 0, render_distance.y, -spacing, &graph_horizontal_line);

	// using actual dimensions for text
	SDL_RenderSetLogicalSize(renderer, window_width_raw, window_height_raw);
	draw_coordinates();

	draw_numbers(0, 1, 0, spacing);
	draw_numbers(0, -1, 0, -spacing);
	draw_numbers(0, 0, 1, spacing);
	draw_numbers(0, 0, -1, -spacing);
	SDL_RenderPresent(renderer);
}

void draw_line(int x, int y, int render_distance_x, int render_distance_y, int spacing, SPRITE* sprite)
{
	int amount = (x > 0) ? render_distance_x : render_distance_y;
	int draw_location = (x > 0) ? x : y;

	for (int count = 0, axis = draw_location + (spacing); count < amount;
		axis += (spacing), count++)
	{
		if(x > 0)
			sprite->rect.x = axis;
		else {
			sprite->rect.y = axis;
		}
		SDL_RenderCopy(renderer, sprite->texture, NULL,
					   &(sprite->rect));
	}
}

void draw_numbers(int count, int distance_x, int distance_y, int _spacing)
{
	int distance = (distance_x != 0) ? distance_x : distance_y;
	int font_size = spacing;

	for (; count < MAX_FONT_COORDINATES; count++, distance += (_spacing))
	{
		SPRITE *d_font = load_number(count, 15);

		if(distance_x != 0)
		{
			d_font->rect.x = (origin.x + origin_offset.x + distance) * (window_width_raw / (PIXEL_WIDTH));
			d_font->rect.y = (origin.y + origin_offset.y) * (window_height_raw / (PIXEL_HEIGHT));
		}
	else
		{
			d_font->rect.x = (origin.x + origin_offset.x) * (window_width_raw / (PIXEL_WIDTH));
			d_font->rect.y = (origin.y + origin_offset.y + distance) * (window_height_raw / (PIXEL_HEIGHT));
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

void mouse_event(void)
{
	if (left_mousedown)
	{
		int x, y;
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
		origin_offset.x += ((first_click_x - x)) / mouse_speed.x;
		origin_offset.y += ((first_click_y - y)) / mouse_speed.y;
	}
}

char num_buffer2[MAX_FONT_COORDINATES];

void draw_coordinates(void)
{
	memset(&coordinate_x, 0, MAX_FONT_COORDINATES - 1);
	memset(&coordinate_y, 0, MAX_FONT_COORDINATES - 1);

	strncpy(coordinate_x, coordinate_str_x, MAX_FONT_COORDINATES - 1);
	strncpy(coordinate_y, coordinate_str_y, MAX_FONT_COORDINATES - 1);

	snprintf(num_buffer2, MAX_FONT_COORDINATES - 1, "%d", (int)-(origin_offset.x));

	char *final_str_x = strncat(coordinate_x, num_buffer2, MAX_FONT_COORDINATES - 1);
	snprintf(num_buffer2, MAX_FONT_COORDINATES - 1, "%d", (int)origin_offset.y);
	char *final_str_y = strncat(coordinate_y, num_buffer2, MAX_FONT_COORDINATES - 1);

	SDL_Texture *texture = NULL;
	SPRITE temp_font_x;
	load_string_font(renderer, &temp_font_x, texture, final_str_x);
	SDL_RenderCopy(renderer, temp_font_x.texture, NULL, &temp_font_x.rect);
	SDL_DestroyTexture(temp_font_x.texture);

	SPRITE temp_font_y;
	load_string_font(renderer, &temp_font_y, texture, final_str_y);
	temp_font_y.rect.y = 75;
	SDL_RenderCopy(renderer, temp_font_y.texture, NULL, &temp_font_y.rect);

	SDL_DestroyTexture(temp_font_y.texture);
}

void input(void)
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
		case SDL_KEYDOWN:

			break;
		case SDL_MOUSEBUTTONDOWN:
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
			break;
		case SDL_MOUSEWHEEL:
			if (e.wheel.y > 0)
			{
				if (spacing >= SPACING_LIMIT)
					spacing = SPACING_LIMIT;
				else
					spacing++;
			}
			else if (e.wheel.y < 0)
			{
				if (spacing <= 1)
					spacing = 1.0f;
				else
					spacing--;
			}

			break;

		case SDL_WINDOWEVENT:

			switch (e.window.event)
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				window_width_raw = e.window.data1;
				window_height_raw = e.window.data2;
				
				if(window_width_raw > window_height_raw)
				{
					orientation = LANDSCAPE;
					PIXEL_WIDTH = 16 * 6;
					PIXEL_HEIGHT = 9 * 6;
				} else
				{
					orientation = PORTRAIT;
					PIXEL_WIDTH = 9 * 6;
					PIXEL_HEIGHT = 16 * 6;
				}

				origin.x = PIXEL_WIDTH / 2;
				origin.y = PIXEL_HEIGHT / 2;

				render_distance.x = PIXEL_WIDTH * 100;
				render_distance.y = PIXEL_HEIGHT * 100;

				break;
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
	*x /= PIXEL_WIDTH;
	*y /= PIXEL_HEIGHT;
}

void conv_fvec(float *x, float *y)
{
	*x /= PIXEL_WIDTH;
	*y /= PIXEL_HEIGHT;
}

void cleanup(void)
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(vertical_line.texture);
	SDL_DestroyTexture(horizontal_line.texture);
	SDL_DestroyTexture(graph_vertical_line.texture);
	SDL_DestroyTexture(graph_horizontal_line.texture);
	font_cleanup();
	printf("CLEANUP\n");
	SDL_Quit();
}
