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
#include <limits.h> /* MAX VALUE FOR GRAPH LONG_MAX */
//#include <unistd.h> // For sleep();

// Font things
#include "font_handler.h"

// Window information for initialization
#define WINDOW_TITLE "Grapher"


// Let's try out same width and height. This will be our 'imaginary dimensions'
// So if screen isn't 16:9.. it looks off. A possible fix would be to just change the imaginary units according to the aspect ratio of the screen
// this will have to take into account screen dimension changes.
#define PIXEL_WIDTH 16*6
#define PIXEL_HEIGHT 9*6

// Sane? don't know how it'll work with larger or smaller screens
#define STARTING_WINDOW_HEIGHT 1920
#define STARTING_WINDOW_WIDTH 1080
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED

#define HORIZONTAL_LINE_WIDTH PIXEL_WIDTH
#define HORIZONTAL_LINE_HEIGHT PIXEL_HEIGHT/16
#define VERTICAL_LINE_WIDTH PIXEL_WIDTH/28
#define VERTICAL_LINE_HEIGHT PIXEL_HEIGHT

#define GRAPH_HORIZONTAL_LINE_WIDTH PIXEL_WIDTH
#define GRAPH_HORIZONTAL_LINE_HEIGHT PIXEL_HEIGHT/16
#define GRAPH_VERTICAL_LINE_WIDTH PIXEL_WIDTH/28
#define GRAPH_VERTICAL_LINE_HEIGHT PIXEL_HEIGHT

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event e;

int window_width_raw = STARTING_WINDOW_WIDTH;
int window_height_raw = STARTING_WINDOW_HEIGHT;

struct FLOAT_VECTOR2 {
	float x;
	float y;
};

struct INT_VECTOR2 {
	int x;
	int y;
};

struct INT_VECTOR2 origin = {PIXEL_WIDTH/2, PIXEL_HEIGHT/2};
struct INT_VECTOR2 origin_offset = {0, 0};
struct FLOAT_VECTOR2 mouse_speed = {211, 152};
struct INT_VECTOR2 coordinate_location = {0, 0};

// Variables for keeping track of how far away we're from the origin
// Used for drawing graph
// Amount of space away from y and x axis
// Used for drawing lines in the quadrants
static float spacing = 2.0f;
#define SPACING_LIMIT 22.0f

struct INT_VECTOR2 render_distance = {PIXEL_WIDTH*100, PIXEL_HEIGHT*100};

bool quit = false;

static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void draw_axes(void);
static void draw_coordinates(void);
static void draw_numbers(void);
static void conv_ivec(int* a, int* b);
static void conv_fvec(float* a, float* b);

char* coordinate_str_x = "Coordinate X: ";
char* coordinate_str_y = "Coordinate Y: ";

// find out limit of an array size(does type matter for array size??)
//#define MAX_COORD_SIZE 1200
char coordinate_x[MAX_FONT_COORDINATES];
char coordinate_y[MAX_FONT_COORDINATES];

float fps;
const float FPS_TARGET = 60.0f;

SPRITE horizontal_line;
SPRITE vertical_line;

SPRITE graph_horizontal_line;
SPRITE graph_vertical_line;

void initialize(void)
{
	printf("Iniitializing\n\n");
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init(renderer);

	char* horizontal_line_path = "/home/johnny/Documents/Projects/SDL/Grapher/horizontal_line.png";
	char* vertical_line_path = "/home/johnny/Documents/Projects/SDL/Grapher/vertical_line.png";
	char* graph_horizontal_line_path = "/home/johnny/Documents/Projects/SDL/Grapher/graph_horizontal_line.png";
	char* graph_vertical_line_path = "/home/johnny/Documents/Projects/SDL/Grapher/graph_vertical_line.png";

	SDL_Surface* surface1 = IMG_Load(horizontal_line_path);
	horizontal_line.texture = SDL_CreateTextureFromSurface(renderer, surface1);
	horizontal_line.rect.w = HORIZONTAL_LINE_WIDTH;
	horizontal_line.rect.h = HORIZONTAL_LINE_HEIGHT;
	horizontal_line.rect.x = 0;
	horizontal_line.rect.y = PIXEL_HEIGHT/2;
	SDL_FreeSurface(surface1);

	SDL_Surface* surface2 = IMG_Load(graph_horizontal_line_path);
	graph_horizontal_line.texture = SDL_CreateTextureFromSurface(renderer, surface2);
	graph_horizontal_line.rect.w = GRAPH_HORIZONTAL_LINE_WIDTH;
	graph_horizontal_line.rect.h = GRAPH_HORIZONTAL_LINE_HEIGHT;
	graph_horizontal_line.rect.x = 0;
	graph_horizontal_line.rect.y = PIXEL_HEIGHT/2;
	SDL_FreeSurface(surface2);

	SDL_Surface* surface3 = IMG_Load(vertical_line_path);
	vertical_line.texture = SDL_CreateTextureFromSurface(renderer, surface3);
	vertical_line.rect.w = VERTICAL_LINE_WIDTH;
	vertical_line.rect.h = VERTICAL_LINE_HEIGHT;
	vertical_line.rect.x = PIXEL_WIDTH/2;
	vertical_line.rect.y = 0;
	SDL_FreeSurface(surface3);

	SDL_Surface* surface4 = IMG_Load(graph_vertical_line_path);
	graph_vertical_line.texture = SDL_CreateTextureFromSurface(renderer, surface4);
	graph_vertical_line.rect.w = GRAPH_VERTICAL_LINE_WIDTH;
	graph_vertical_line.rect.h = GRAPH_VERTICAL_LINE_HEIGHT;
	graph_vertical_line.rect.x = PIXEL_WIDTH/2;
	graph_vertical_line.rect.y = 0;
	SDL_FreeSurface(surface4);


	conv_fvec(&mouse_speed.x, &mouse_speed.y);
}

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
		char buffconv[MAX_FONT_COORDINATES] = {0};
		strncat(buffconv, "FPS: ", MAX_FONT_COORDINATES-1);
		snprintf(&buffconv[5], MAX_FONT_COORDINATES-5, "%d", (int)fps);
		SDL_Texture* texture = NULL;
		struct SPRITE* temp = load_string_font(renderer, texture, buffconv);
		temp->rect.x = 0;
		temp->rect.y = 200;

		SDL_RenderCopy(renderer, temp->texture, NULL, &temp->rect);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(temp->texture);

		//printf("Printing fps: %d\n", (int)fps);

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

	// axes in imaginary size so it's scaled up or down depending on actual dimensions
	SDL_RenderSetLogicalSize(renderer, PIXEL_WIDTH, PIXEL_HEIGHT);
	draw_axes();
	// using actual dimensions for text
	SDL_RenderSetLogicalSize(renderer, window_width_raw, window_height_raw);
	draw_coordinates();
	draw_numbers();
	SDL_RenderPresent(renderer);
}

bool mouse_moved = false;
bool mouse_held = false;
bool first_click = false;
int first_click_x = 0;
int first_click_y = 0;
Uint32 buttons;
bool left_mousedown = false;

void conv_ivec(int* x, int* y)
{
	*x/=PIXEL_WIDTH;
	*y/=PIXEL_HEIGHT;
}

void conv_fvec(float* x, float* y)
{
	*x/=PIXEL_WIDTH;
	*y/=PIXEL_HEIGHT;
}

void draw_axes(void)
{
	coordinate_location.x = origin_offset.x + origin.x;
	coordinate_location.y = origin_offset.y + origin.y;

	for(int count = 0, y = coordinate_location.y + (-spacing); count < render_distance.y; y += -spacing, count++)
	{
		graph_horizontal_line.rect.y = y;
		SDL_RenderCopy(renderer, graph_horizontal_line.texture, NULL, &(graph_horizontal_line.rect));
	}

	for(int count = 0, y = coordinate_location.y + (spacing); count < render_distance.y; y += spacing, count++)
	{
		graph_horizontal_line.rect.y = y;
		SDL_RenderCopy(renderer, graph_horizontal_line.texture, NULL, &(graph_horizontal_line.rect));
	}

	for(int count = 0, x = coordinate_location.x + (-spacing); count < render_distance.x; x += -spacing, count++)
	{
		graph_vertical_line.rect.x = x;
		SDL_RenderCopy(renderer, graph_vertical_line.texture, NULL, &(graph_vertical_line.rect));
	}

	for(int count = 0, x = coordinate_location.x + (spacing); count < render_distance.x; x += spacing, count++)
	{
		graph_vertical_line.rect.x = x;
		SDL_RenderCopy(renderer, graph_vertical_line.texture, NULL, &(graph_vertical_line.rect));
	}

	vertical_line.rect.x = coordinate_location.x;
	SDL_RenderCopy(renderer, vertical_line.texture, NULL, &(vertical_line.rect));

	horizontal_line.rect.y = coordinate_location.y;
	SDL_RenderCopy(renderer, horizontal_line.texture, NULL, &(horizontal_line.rect));
}

void draw_numbers(void)
{
	struct INT_VECTOR2 distance;
	int count;
	//old code - limits numbers to distance
	//for(distance.y = 0, distance.x = 0;
		//count < render_distance.y; distance.y += (-spacing), count++)

	// doesn't limit numbers on distance but on MAX_COORD_SIZE
	// Unfortunately cuts FPS by half but that is to be expected since it does render more numbers.
	// NEED optimization to only render numbers on screen
	// ALSO future problem. When user zooms all the way out and all the coordinates are shown... might cause drastic slow down.
	// Limit zoom or find a better way to render all those coordinates.

	/*
	// Positive up + 0
	int eval = (origin_offset.y+PIXEL_HEIGHT/2);
	for(count = 0, distance.y = 0, distance.x = 0; count < MAX_FONT_COORDINATES; count++, distance.y += (-spacing))
	{
		int count_spacing = count*spacing;
		if(count_spacing <= (eval) && (count_spacing) > origin_offset.y-origin.y)
		{
			SPRITE* r_font = load_number(count);
			r_font->rect.x = distance.x + coordinate_location.x;
			r_font->rect.y = distance.y + coordinate_location.y;

			SDL_RenderCopy(renderer, r_font->texture, NULL, &(r_font->rect));
		}
	}

	// negatives down
	for(count = 1, distance.y = (spacing), distance.x = 0; count < MAX_FONT_COORDINATES; count++, distance.y += (spacing))
	{
		int count_spacing = count*spacing;
		if(-count_spacing <= -(eval) && -(count_spacing) > -origin_offset.x+origin.x)
		{
			SPRITE* r_font = load_number(count);

			r_font->rect.x = distance.x + coordinate_location.x;
			r_font->rect.y = distance.y + coordinate_location.y;

		SDL_RenderCopy(renderer, r_font->texture, NULL, &(r_font->rect));
		}
	}
	*/

	// positive right
	for(count = 0, distance.y = 0, distance.x = (spacing*2); count < MAX_FONT_COORDINATES; count++, distance.x += (spacing))


	{
		SPRITE* d_font = load_number(count, spacing);

		d_font->rect.x = (origin.x + origin_offset.x + distance.x)*(window_width_raw/(PIXEL_WIDTH));
		d_font->rect.y = (origin.y + origin_offset.y)*(window_height_raw/(PIXEL_HEIGHT));
		SDL_RenderCopy(renderer, d_font->texture, NULL, &(d_font->rect));
	}

	// negatives left
	// This is one is a bit different. Don't know why we have to specifically assign -1 to count and not on the negative down for loop
	for(count = 1, distance.y = 0, distance.x = 2; count < MAX_FONT_COORDINATES; count++, distance.x += (-spacing))
	{
		SPRITE* d_font = load_number(-count, 12);
		d_font->rect.x = (origin.x + origin_offset.x + distance.x)*(window_width_raw/(PIXEL_WIDTH));
		d_font->rect.y = (origin.y + origin_offset.y)*(window_height_raw/(PIXEL_HEIGHT));
		SDL_RenderCopy(renderer, d_font->texture, NULL, &(d_font->rect));
	}

}

void mouse_event(void)
{
	if(left_mousedown)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		conv_ivec(&x, &y);

		if(first_click == false)
		{
			first_click = true;
			first_click_x = x;
			first_click_y = y;

			printf("Changing clickclickclick\n\n");
		}
		printf("X: %d\n", x);
		printf("Y: %d\n", y);
		// We get the difference between the coordinates from when
		// the user first clicks the mouse and while they are moving the mouse

		origin_offset.x += ((first_click_x -x)) / mouse_speed.x; //* (fps/1000));
		origin_offset.y += ((first_click_y - y)) / mouse_speed.y; // * (fps/1000));
		//printf("Moving mouse: x: %d   y: %d\n", origin_offset.x, origin_offset.y);
		//sleep(1);
	}

}
/**/
char num_buffer2[MAX_FONT_COORDINATES];

void draw_coordinates(void)
{
	memset(&coordinate_x, 0, MAX_FONT_COORDINATES-1);
	memset(&coordinate_y, 0, MAX_FONT_COORDINATES-1);

	strncpy(coordinate_x, coordinate_str_x, MAX_FONT_COORDINATES-1);
	strncpy(coordinate_y, coordinate_str_y, MAX_FONT_COORDINATES-1);

	snprintf(num_buffer2, MAX_FONT_COORDINATES-1, "%d", (int)-(origin_offset.x));

	// Maybe buffer overflow because string can go MAX_COORD_SIZE + coordinate_x??
	// or maybe not cuz strncat limits it
	char* final_str_x = strncat(coordinate_x, num_buffer2, MAX_FONT_COORDINATES-1);
	snprintf(num_buffer2, MAX_FONT_COORDINATES-1, "%d", (int)origin_offset.y);
	char* final_str_y = strncat(coordinate_y, num_buffer2, MAX_FONT_COORDINATES-1);

	SDL_Texture* texture = NULL;
	SPRITE* temp_font_x = load_string_font(renderer, texture, final_str_x);
	SDL_RenderCopy(renderer, temp_font_x->texture, NULL, &temp_font_x->rect);
	SDL_DestroyTexture(temp_font_x->texture);

	SPRITE* temp_font_y = load_string_font(renderer, texture, final_str_y);
	temp_font_y->rect.y = 75;
	SDL_RenderCopy(renderer, temp_font_y->texture, NULL, &temp_font_y->rect);

	SDL_DestroyTexture(temp_font_y->texture);
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
				mouse_moved = true;
				if((e.type & SDL_BUTTON_LMASK) != 0)
				{
					left_mousedown = true;
				}
				if((buttons & SDL_BUTTON_LMASK) != 0)
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
					//if((e.type & SDL_BUTTON_LMASK) != 0)
				//	{
						first_click_x = 0;
						first_click_y = 0;

						first_click = false;
						printf("MOUSE GOING UP\n");
					//}



					break;

			case SDL_FINGERUP:
				left_mousedown = false;
				mouse_moved = false;
				first_click_x = 0;
				first_click_y = 0;
				break;
			case SDL_MOUSEWHEEL:
				if(e.wheel.y > 0)
				{
					if(spacing >= SPACING_LIMIT)
						spacing = SPACING_LIMIT;
					else
						spacing++;
				}
				else if(e.wheel.y < 0)
				{
					if(spacing <= 1)
						spacing = 1.0f;
					else spacing--;
				//	} else spacing = SPACING_LIMIT;
				}

				break;

			case SDL_WINDOWEVENT:

				switch(e.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						/*
						** Change line coordinates to new window size
						*/
						//printf("First time called\n");
						//sleep(5);
						window_width_raw = e.window.data1;
						window_height_raw = e.window.data2;
						printf("Window width raw: %d\nWindow Height raw: %d \n", window_width_raw, window_height_raw);

						//origin.x = window_width_raw/2;
						//origin.y = window_height_raw/2;

						//render_distance.x = window_width_raw/spacing;
						//render_distance.y = window_height_raw/spacing;
						//render_distance.x = window_width_raw/spacing/PIXEL_WIDTH;
						//render_distance.y = window_height_raw/spacing/PIXEL_HEIGHT;

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
	SDL_DestroyTexture(vertical_line.texture);
	SDL_DestroyTexture(horizontal_line.texture);
	SDL_DestroyTexture(graph_vertical_line.texture);
	SDL_DestroyTexture(graph_horizontal_line.texture);
	font_cleanup();
	printf("CLEANUP\n");
	SDL_Quit();
}
