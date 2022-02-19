/* TODO
**
**  1. Pixel independent world units. This is needed to add numbers to graph, scrolling, coordinates and will increase portability.
**  2. Implement a constant frame-rate loop. Possibly 30-60 fps. This will help with smoothness of mouse gestures.
**  3. Fix mouse gesture when swiping(currently inconsistent)
**  4. Implement actual coordinates for entire graph.
*/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <unistd.h> // For sleep();

// Font things
#include "font_handler.h"

// Window information for initialization
#define WINDOW_TITLE "Grapher"
#define STARTING_WINDOW_HEIGHT 1920
#define STARTING_WINDOW_WIDTH 1080
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event e;

int window_width = STARTING_WINDOW_WIDTH;
int window_height = STARTING_WINDOW_HEIGHT;

struct INT_VECTOR2 {
	int x;
	int y;
};

struct FLOAT_VECTOR2 {
	float x;
	float y;
};

struct INT_VECTOR2 origin = {STARTING_WINDOW_WIDTH/2, STARTING_WINDOW_HEIGHT/2};
struct INT_VECTOR2 origin_offset;
struct FLOAT_VECTOR2 mouse_speed = {5, 3};
struct INT_VECTOR2 coordinate_location;

// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
// Amount of space away from y and x axis
// Used for drawing lines in the quadrants
static float spacing = 40.0f;
#define SPACING_LIMIT 25.0f

struct INT_VECTOR2 render_distance = {STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT};

bool quit = false;

static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void draw_axes(void);
static void draw_coordinates(void);
static void draw_numbers(void);

char* coordinate_str_x = "Coordinate X: ";
char* coordinate_str_y = "Coordinate Y: ";

// find out limit of an array size(does type matter for array size??)
#define MAX_COORD_SIZE 120
char coordinate_x[MAX_COORD_SIZE];
char coordinate_y[MAX_COORD_SIZE];

float fps;
const float FPS_TARGET = 60.0f;

int main(void)
{
	initialize();
	//ADAPTIVE SYNC (-1) IMMEDIATE(0)
	SDL_GL_SetSwapInterval(1);

	while(!quit)
	{
		// Ripped of internet fps counter
		Uint32 start_time = SDL_GetTicks();
		Uint32 frame_time;
		//float fps;

		input();
		draw();

		frame_time = SDL_GetTicks() - start_time;


		fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
		// Draw fps
		char buffconv[MAX_COORD_SIZE] = {0};
		strncat(buffconv, "FPS: ", MAX_COORD_SIZE-1);
		snprintf(&buffconv[5], MAX_COORD_SIZE-5, "%d", (int)fps);
		SDL_Texture* texture;
		struct temp_font temp = load_string_font(renderer, texture, buffconv);
		temp.rect.x = 200;
		temp.rect.y = 300;

		SDL_RenderCopy(renderer, temp.texture, NULL, &temp.rect);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(temp.texture);
		printf("Printing fps: %d\n", (int)fps);

		//if(fps > FPS_TARGET)
			//SDL_Delay((fps-FPS_TARGET)/1000);

	}

	cleanup();

	return 0;
}

void draw(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	draw_axes();
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

	for(int count = 0, y = coordinate_location.y + (-spacing); count < render_distance.y; y += -spacing, count++)
	{
		SDL_RenderDrawLine(renderer, 0, y, window_width, y);
	}

	for(int count = 0, y = coordinate_location.y + (spacing); count < render_distance.y; y += spacing, count++)
	{
		SDL_RenderDrawLine(renderer, 0, y, window_width, y);
	}

	for(int count = 0, x = coordinate_location.x + (-spacing); count < render_distance.x; x += -spacing, count++)
	{
		SDL_RenderDrawLine(renderer, x, 0,  x, window_height);
	}

	for(int count = 0, x = coordinate_location.x + (spacing); count < render_distance.x; x += spacing, count++)
	{
		SDL_RenderDrawLine(renderer, x, 0, x, window_height);

	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_RenderDrawLine(renderer, 0, coordinate_location.y, window_width, coordinate_location.y);
	SDL_RenderDrawLine(renderer, coordinate_location.x, 0, coordinate_location.x, window_height);

	draw_numbers();
}

void draw_numbers(void)
{
	struct INT_VECTOR2 distance;
	// positives up + 0
	char buffconv[MAX_COORD_SIZE];
	struct temp_font temp;
	int count;
	//int count = 0;
	//old code - limits numbers to distance
	//for(distance.y = 0, distance.x = 0;
		//count < render_distance.y; distance.y += (-spacing), count++)

	// doesn't limit numbers on distance but on MAX_COORD_SIZE
	/* Unfortunately cuts FPS by half but that is to be expected since it does render more numbers.
	** NEED optimization to only render numbers on screen
	** ALSO future problem. When user zooms all the way out and all the coordinates are shown... might cause drastic slow down.
	** Limit zoom or find a better way to render all those coordinates.
	**
	**
	*/
	int eval = (origin_offset.y+window_height/2);
	for(count = 0, distance.y = 0, distance.x = 0; count < MAX_COORD_SIZE; count++, distance.y += (-spacing))
	{
		int count_spacing = count*spacing;
		if(count_spacing <= (eval) && (count_spacing) > origin_offset.y-origin.y)
		{
			//printf("COUNT*SPACE: %lf <= EVAL: %d\n", count*spacing, eval);
			snprintf(buffconv, MAX_COORD_SIZE, "%d", count);
			SDL_Texture* texture;
			temp = load_string_font(renderer, texture, buffconv);
			temp.rect.x = distance.x + coordinate_location.x;
			temp.rect.y = distance.y + coordinate_location.y;

			SDL_RenderCopy(renderer, temp.texture, NULL, &temp.rect);
			SDL_DestroyTexture(temp.texture);
		}
	}
	//count = 1;
	// negatives down
	//for(distance.y = (spacing), distance.x = 0; count < render_distance.y; distance.y += spacing, count++){
	for(count = 1, distance.y = (spacing), distance.x = 0; count < MAX_COORD_SIZE; count++, distance.y += (spacing))
	{
		int count_spacing = count*spacing;
		if(-count_spacing <= -(eval) && -(count_spacing) > -origin_offset.x+origin.x)
		{
		snprintf(buffconv, MAX_COORD_SIZE, "%d", -count);
		//temp = load_string_font(renderer, buffconv);
		SDL_Texture* texture;
		temp = load_string_font(renderer, texture, buffconv);
		temp.rect.x = distance.x + coordinate_location.x;
		temp.rect.y = distance.y + coordinate_location.y;

		SDL_RenderCopy(renderer, temp.texture, NULL, &temp.rect);
		SDL_DestroyTexture(temp.texture);
		}
	}

	//count = 1;
	// positive right
	//for(distance.y = 0, distance.x = (spacing); count < render_distance.x; distance.x += spacing, count++){
	for(count = 1, distance.y = 0, distance.x = (spacing); count < MAX_COORD_SIZE; count++, distance.x += (spacing))
	{
		snprintf(buffconv, MAX_COORD_SIZE, "%d", count);
		//temp = load_string_font(renderer, buffconv);
		SDL_Texture* texture;
		temp = load_string_font(renderer, texture, buffconv);
		temp.rect.x = distance.x + coordinate_location.x;
		temp.rect.y = distance.y + coordinate_location.y;

		SDL_RenderCopy(renderer, temp.texture, NULL, &temp.rect);
		SDL_DestroyTexture(temp.texture);
	}

	//count = -1;
	// negatives left
	// This is one is a bit different. Don't know why we have to specifically assign -1 to count and not on the negative down for loop
	//for(distance.y = 0, distance.x = (spacing); count < render_distance.x; distance.x += (-spacing), count++)
	for(count = -1, distance.y = 0, distance.x = (spacing); count < MAX_COORD_SIZE; count++, distance.x += (-spacing))
	{
		snprintf(buffconv, MAX_COORD_SIZE, "%d", -count);
		//temp = load_string_font(renderer, buffconv);
		SDL_Texture* texture;
		temp = load_string_font(renderer, texture, buffconv);
		temp.rect.x = distance.x + coordinate_location.x;
		temp.rect.y = distance.y + coordinate_location.y;

		SDL_RenderCopy(renderer, temp.texture, NULL, &temp.rect);
		SDL_DestroyTexture(temp.texture);
	}
}


void mouse_event(void)
{
	if(left_mousedown)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);

		if(first_click == false)
		{
			first_click = true;
			first_click_x = x;
			first_click_y = y;

			first_click = true;
		}
		coordinate_location.x = x;
		coordinate_location.y = y;

		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse
		origin_offset.x += ((first_click_x -x) * (fps/1000));
		origin_offset.y += ((first_click_y - y) * (fps/1000));
	}

}

char num_buffer[MAX_COORD_SIZE];

void draw_coordinates(void)
{
	memset(&coordinate_x, 0, MAX_COORD_SIZE-1);
	memset(&coordinate_y, 0, MAX_COORD_SIZE-1);

	strncpy(coordinate_x, coordinate_str_x, MAX_COORD_SIZE-1);
	strncpy(coordinate_y, coordinate_str_y, MAX_COORD_SIZE-1);

	snprintf(num_buffer, MAX_COORD_SIZE, "%d", (int)-(origin_offset.x));

	// Maybe buffer overflow because string can go MAX_COORD_SIZE + coordinate_x??
	// or maybe not cuz strncat limits it
	char* final_str_x = strncat(coordinate_x, num_buffer, MAX_COORD_SIZE-1);
	snprintf(num_buffer, MAX_COORD_SIZE, "%d", (int)origin_offset.y);
	char* final_str_y = strncat(coordinate_y, num_buffer, MAX_COORD_SIZE-1);

	SDL_Texture* texture;
	struct temp_font temp_font_x = load_string_font(renderer, texture, final_str_x);
	SDL_RenderCopy(renderer, temp_font_x.texture, NULL, &temp_font_x.rect);
	SDL_DestroyTexture(temp_font_x.texture);

	struct temp_font temp_font_y = load_string_font(renderer, texture, final_str_y);
	temp_font_y.rect.y = 75;
	SDL_RenderCopy(renderer, temp_font_y.texture, NULL, &temp_font_y.rect);

	SDL_DestroyTexture(temp_font_y.texture);
}

void initialize(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	//SDL_RenderSetScale(renderer, 16, 9);
	//SDL_RenderSetScale(renderer, 2, 2);
	//SDL_RenderSetLogicalSize(renderer, 4*100, 3*100);

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
				if((e.type & SDL_BUTTON_LMASK) != 0)
				{
					left_mousedown = true;
				}
				if((buttons & SDL_BUTTON_LMASK) != 0)
				{
				}
				break;

			case SDL_MOUSEBUTTONUP:
				left_mousedown = false;
					if((e.type & SDL_BUTTON_LMASK) != 0)
					{
						first_click_x = 0;
						first_click_y = 0;

						first_click = false;
					}


					break;

			case SDL_MOUSEWHEEL:
				if(e.wheel.y > 0)
				{
					spacing++;
				}
				else if(e.wheel.y < 0)
				{
					if(spacing > SPACING_LIMIT)
					{
						spacing--;
					} else spacing = SPACING_LIMIT;
				}

				break;

			case SDL_WINDOWEVENT:

				switch(e.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						/*
						** Change line coordinates to new window size
						*/
						window_width = e.window.data1;
						window_height = e.window.data2;

						origin.x = window_width/2;
						origin.y = window_height/2;

						render_distance.x = window_width/spacing;
						render_distance.y = window_height/spacing;

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
