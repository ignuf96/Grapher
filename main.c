// hello comment
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

// Font things
#include "font_handler.h"

// Window information for initialization
#define WINDOW_TITLE "Grapher"
#define STARTING_WINDOW_HEIGHT 1920
#define STARTING_WINDOW_WIDTH 1080
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED

#define ORIGIN_OFFSET_X 0
#define ORIGIN_OFFSET_Y 1

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Renderer* font_renderer;
SDL_Event e;

int window_width = STARTING_WINDOW_WIDTH;
int window_height = STARTING_WINDOW_HEIGHT;

struct INT_VECTOR2 {
	int x;
	int y;
};

struct INT_VECTOR2 origin = {STARTING_WINDOW_WIDTH/2, STARTING_WINDOW_HEIGHT/2};
struct INT_VECTOR2 origin_offset;
struct INT_VECTOR2 mouse_speed = {200, 120};
struct INT_VECTOR2 coordinate_location;

// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
// Amount of space away from y and x axis
// Used for drawing lines in the quadrants
static float spacing = 1.0f;

static int render_distance = 1000;

bool quit = false;

void initialize(void);
void input(void);
void cleanup(void);
void mouse_event(void);
void draw(void);
void draw_axes(void);
void draw_coordinates(void);

char* coordinate_str_x = "Coordinate X: ";
char* coordinate_str_y = "Coordinate Y: ";

// find out limit of an array size(does type matter for array size??)
#define MAX_COORD_SIZE 20000
char coordinate_x[MAX_COORD_SIZE];
char coordinate_y[MAX_COORD_SIZE];

int main(void)
{

	initialize();
	SDL_GL_SetSwapInterval(1);

	load_fonts(renderer, "OpenSans-Regular.ttf");

	while(!quit)
	{
		input();
		draw();
	}

	cleanup();

	return 0;
}

void draw(void)
{
	SDL_RenderSetScale(renderer, 2, 2);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	draw_axes();

	font_draw(renderer, 22, NUMBER_0, (origin.x+origin_offset.x/mouse_speed.x) + 5, (origin.y+origin_offset.y/mouse_speed.y));

	SDL_RenderSetScale(renderer, 1, 1);
	draw_coordinates();

	SDL_RenderPresent(renderer);
}

bool mouse_held = false;
bool first_click = false;
int first_click_x = 0;
int first_click_y = 0;
Uint32 buttons;
bool left_mousedown = false;

void draw_axes(void)
{

	coordinate_location.x = origin.x+origin_offset.x/mouse_speed.x;
	coordinate_location.y = origin.y+origin_offset.y/mouse_speed.y;

	SDL_SetRenderDrawColor(renderer, 145, 145, 145, 255);

	for(int count = 0, y = coordinate_location.y + (-spacing); count < render_distance; y += -spacing, count++)
	{
		SDL_RenderDrawLine(renderer, 0, y, window_width, y);
	}

	for(int count = 0, y = coordinate_location.y + (spacing); count < render_distance; y += spacing, count++)
	{
		SDL_RenderDrawLine(renderer, 0, y, window_width, y);
	}

	for(int count = 0, x = coordinate_location.x + (-spacing); count < render_distance; x += -spacing, count++)
	{
		SDL_RenderDrawLine(renderer, x, 0, x, window_height);
	}

	for(int count = 0, x = coordinate_location.x + (spacing); count < render_distance; x += spacing, count++)
	{
		SDL_RenderDrawLine(renderer, x, 0, x, window_height);
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_RenderDrawLine(renderer, 0, coordinate_location.y, window_width, coordinate_location.y);
	SDL_RenderDrawLine(renderer, coordinate_location.x, 0, coordinate_location.x, window_height);


}

void mouse_event(void)
{
	if(left_mousedown)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);

		printf("Left Mouse Down\n");
		if(first_click == false)
		{
			printf("Left Mouse first click\n");
			first_click = true;
			first_click_x = x;
			first_click_y = y;

			printf("First Click x:%d\ny:%d\n", first_click_x, first_click_y);

			first_click = true;
		}
		printf("X: %d\nY:%d\n", x, y);
		coordinate_location.x = x;
		coordinate_location.y = y;

		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse
		origin_offset.x += (first_click_x -x);
		origin_offset.y += (first_click_y - y);
	}

}

char num_buffer[MAX_COORD_SIZE];

void draw_coordinates(void)
{
	memset(&coordinate_x, 0, MAX_COORD_SIZE-1);
	memset(&coordinate_y, 0, MAX_COORD_SIZE-1);

	strncpy(coordinate_x, coordinate_str_x, MAX_COORD_SIZE-1);
	strncpy(coordinate_y, coordinate_str_y, MAX_COORD_SIZE-1);

	snprintf(num_buffer, MAX_COORD_SIZE, "%d", coordinate_location.x);

	// Maybe buffer overflow because string can go MAX_COORD_SIZE + coordinate_x??
	// or maybe not cuz strncat limits it
	char* final_str_x = strncat(coordinate_x, num_buffer, MAX_COORD_SIZE-1);
	snprintf(num_buffer, MAX_COORD_SIZE, "%d", coordinate_location.y);
	char* final_str_y = strncat(coordinate_y, num_buffer, MAX_COORD_SIZE-1);

	printf("Final String X: %s\n", final_str_x);
	printf("Final String Y: %s\n", final_str_y);

	struct temp_font temp_font_x = load_string_font(renderer, "OpenSans-Regular.ttf", final_str_x);
	SDL_RenderCopy(renderer, temp_font_x.texture, NULL, &temp_font_x.rect);

	struct temp_font temp_font_y = load_string_font(renderer, "OpenSans-Regular.ttf", final_str_y);
	temp_font_y.rect.y = 75;
	SDL_RenderCopy(renderer, temp_font_y.texture, NULL, &temp_font_y.rect);

	SDL_DestroyTexture(temp_font_x.texture);
	SDL_DestroyTexture(temp_font_y.texture);
}

void initialize(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_RenderSetScale(renderer, 16, 9);

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init();

}

void input(void)
{
	quit = false;

	while(SDL_PollEvent(&e))
	{
		switch(e.type)
		{
			case SDL_QUIT:
				quit = true;
				cleanup();
				break;
			case SDL_KEYDOWN:

				break;

			case SDL_MOUSEBUTTONDOWN:
				if(e.type & SDL_BUTTON_LMASK != 0)
				{
					left_mousedown = true;
				}
				printf("A Mouse button is down\n");
				if(buttons & SDL_BUTTON_LMASK != 0)
				{
				}
				break;

			case SDL_MOUSEBUTTONUP:
				printf("Button up!!!\n");
				left_mousedown = false;
					//if(e.type & SDL_BUTTON_LMASK != 0)
					//{
						printf("Left mouse disengaged\n");

						first_click_x = 0;
						first_click_y = 0;

						first_click = false;
					//}


					break;

			case SDL_MOUSEWHEEL:
				if(e.wheel.y > 0)
				{
					spacing++;
				}
				else if(e.wheel.y < 0)
				{
					if(spacing > 0)
					{
						spacing--;
						printf("Spacing: %d\n", spacing);
					} else spacing = 0;
				}

				break;

			case SDL_WINDOWEVENT:

				switch(e.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						/*
						** Change line coordinates to new window size
						*/
						printf("Window size changed to %dx%d\n", e.window.data1, e.window.data2);

						window_width = e.window.data1;
						window_height = e.window.data2;

						origin.x = window_width/2;
						origin.y = window_height/2;

						break;
				}
				break;

		}
	}

	mouse_event();
	SDL_PumpEvents();
}

void cleanup(void)
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	font_cleanup();
	SDL_Quit();
}
