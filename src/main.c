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

// GUI stuff
#include "../include/kiss_sdl.h"

// Window information for initialization
SDL_Window *window;
#define WINDOW_TITLE "Grapher"
#define WINDOW_POSX SDL_WINDOWPOS_UNDEFINED
#define WINDOW_POSY SDL_WINDOWPOS_UNDEFINED

// dimensions 16:9
static int axes_horizontal_line_width, axes_horizontal_line_height;
static int axes_vertical_line_width, axes_vertical_line_height;
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
static const float FPS_TARGET = 240.0f;

static SPRITE axes_horizontal_line;
static SPRITE axes_vertical_line;

static enum DIRECTION{D_UP, D_DOWN, D_LEFT, D_RIGHT, D_NONE}direction=D_NONE;

static int mouse_x, mouse_y;

#define MAX_STR_BUFFER 100
static char str_buffer[100 - 1];
static char str_num_buffer[MAX_STR_BUFFER];

static bool quit = false;
static bool paused = false;
int kissquit = 0;

// declarations
static void initialize(void);
static void input(void);
static void cleanup(void);
static void mouse_event(void);
static void draw(void);
static void update(void);
static void draw_coordinates(void);
static void draw_numbers(void);
static void create_sprite(SPRITE* sprite, char* path, int width, int height, int x, int y);
static void draw_mouse_coordinates(int x, int y);
static int get_number(int num, int part_size);
static void init_box(void);
static void get_quadrant_pos(int location, ivec2 *quadrant, ivec2 *distance);
static void draw_points(void);

void button_event(kiss_button *button, SDL_Event *e, int *draw, int *kissquit)
{
	if(kiss_button_event(button, e, draw))
	{
		*kissquit = 1;
		quit = true;
	}
}

SPRITE graph_box;
SPRITE dot_texture;
SPRITE highlighted_dot_texture;
SPRITE background;

#define NUMBER_OF_QUADRANTS 4

// has to be same width and height or else causes bug
#define GRAPH_WIDTH 100
#define GRAPH_HEIGHT 100

SDL_Rect graph[NUMBER_OF_QUADRANTS][GRAPH_WIDTH][GRAPH_HEIGHT];
SDL_Rect *graphp = &graph[0][0][0];
SDL_Rect graph_size = {0, 0, 0, 0};

struct POINTS {
	SDL_Rect rect;
	bool is_visible;
	bool is_highlighted;
	ivec2 pos;
};

kiss_array objects, a1;
kiss_window kisswindow;
kiss_button button = {0};
kiss_label label = {0};

kiss_entry entry = {0};
int entry_width = 250;
kiss_textbox textbox = {0};
int textbox_width = 250;
int textbox_height = 250;

char message[KISS_MAX_LENGTH];
int kissdraw;

struct POINTS points[NUMBER_OF_QUADRANTS][GRAPH_WIDTH][GRAPH_HEIGHT] = {0, 0, 0};

void initialize(void)
{
	printf("Iniitializing\n\n");
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POSX, WINDOW_POSY, 1920,
							  1080, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	kissdraw = 1;
	kiss_array_new(&objects);

	kiss_init(window, renderer, &objects);

	kiss_window_new(&kisswindow, NULL, 0, 0, 0, 300, 50);
	strcpy(message, "Write Your Linear Equation Here");
	kiss_label_new(&label, &kisswindow, message, 0,
					(kiss_textfont.fontheight + 2*kiss_normal.h) /2);

	label.textcolor.r = 166;
	kisswindow.visible = 1;
	kiss_entry_new(&entry, &kisswindow, 1, "y=mx+b", 0, 0, 300);

	init_world(window);

	starting_spacing = .1f;
	spacing = starting_spacing;
	spacing_limit = 300;

	create_sprite(&background, "../assets/background.png", get_world()->screen_dimensions.y, get_world()->screen_dimensions.y, 0, 0);
	create_sprite(&dot_texture, "../assets/dot.png", 1000, 1000, 0, 0);
	create_sprite(&highlighted_dot_texture, "../assets/highlighteddot.png", 1000, 1000, 0, 0);

	int buffering;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &buffering);

	font_init(renderer);
	init_box();
}

int main(int argc, char **argv)
{
	initialize();
	// ADAPTIVE SYNC (-1) IMMEDIATE(0)
	SDL_GL_SetSwapInterval(1);

	Uint32 time_step_ms = 1000 / 240;
	Uint32 next_game_step = SDL_GetTicks();

	while (!quit)
	{
		while(!paused)
		{
		Uint32 now = SDL_GetTicks();

		if(next_game_step <= now)
		{
			int computer_is_too_slow_limit = 35;

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
		input();
		SDL_Delay(1);
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
				graph[n][i][j].x = x + origin_offset.x+graph_size.x;
				graph[n][i][j].y = y + origin_offset.y+graph_size.y;

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
				if(points[n][i][j].is_visible && !points[n][i][j].is_highlighted)
				{
					SDL_RenderCopy(renderer, dot_texture.texture, NULL, &points[n][i][j].rect);
				}
				else if(points[n][i][j].is_visible && points[n][i][j].is_highlighted)
				{
					SDL_RenderCopy(renderer, highlighted_dot_texture.texture, NULL, &points[n][i][j].rect);
				}
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
	draw_numbers();

	kiss_window_draw(&kisswindow, renderer);
	kiss_label_draw(&label, renderer);
	//kiss_button_draw(&button, renderer);
	kiss_entry_draw(&entry, renderer);
	kiss_textbox_draw(&textbox, renderer);
	kissdraw = 0;
}

void draw_numbers()
{
	int font_size = MIN_FONT_SIZE;
	SPRITE *d_font = load_texture(font_size);
	//int line = starting_line;

	for(int i = MIN_FONT_SIZE; i < MAX_FONT_SIZE; i++)
	{
		SPRITE *d_font = load_texture(i);

		if((d_font->rect.w*2) == spacing)
		{
			font_size = i;
		}
	}

	int part_size = d_font->rect.w / 10;
	bool is_negative;
	#define NUMBER_OF_AXES 4
	for(int n=0; n < NUMBER_OF_AXES; n++)
	{
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

		dest.w = part_size;
		dest.h = d_font->rect.h;

		switch(n)
		{
			case 0:
				direction = D_RIGHT;
				is_negative = false;
				dest.x += (get_world()->world_dimensions.x);
				break;
			case 1:
				direction = D_UP;
				is_negative = false;
				dest.y += -(get_world()->world_dimensions.y);
				break;
			case 2:
				direction = D_LEFT;
				is_negative = true;
				//dest.x += -(graph[0][0][0].w);
				dest.x += -(get_world()->world_dimensions.x);
				break;
			case 3:
				direction = D_DOWN;
				is_negative = true;
				dest.y += (get_world()->world_dimensions.y);
				break;
		}

		for(int i=1; i < GRAPH_WIDTH; i++)
		{
			int divisor = 10;
			int distance_left = get_number(i, 0);

			int distance = distance_left;
			int place = 0;
			int j=i;
			SDL_Rect dest_num = dest;

			//dest.x += direction == D_LEFT || direction == D_DOWN ? (part_size*distance_left) : part_size*distance_left;
			dest.x += (part_size*distance_left);

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
			switch (direction)
			{
				case D_LEFT:
					dest.x += (distance_left - get_world()->world_dimensions.x);
					break;
				case D_RIGHT:
					dest.x += (distance_left + get_world()->world_dimensions.x);
					break;
				case D_DOWN:
					dest.y += (distance_left + get_world()->world_dimensions.y);
					break;
				case D_UP:
					dest.y += (distance_left - get_world()->world_dimensions.y);
					break;
					case D_NONE:
						break;
			}
		}
	}
	//dest.x+= part_size;
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
bool double_click = false;
bool single_click = false;

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
							SDL_Rect temp_rect = points[n][i][j].rect;
							temp_rect.w*=1.5;
							temp_rect.h*=1.5;
							//temp_rect.x/=2;
							//temp_rect.y/=2;

							if(SDL_PointInRect(&point, &temp_rect))
							{
								if(!points[n][i][j].is_visible && single_click && !double_click && !points[n][i][j].is_highlighted)
								{
									points[n][i][j].is_visible = true;
									//single_click = false;
								}
								else if(points[n][i][j].is_visible && !points[n][i][j].is_highlighted && single_click && !double_click)
								{
									points[n][i][j].is_highlighted = true;
									//single_click = false;
								}

								else if(points[n][i][j].is_visible && double_click)
								{
									printf("Hello double click\n");
									points[n][i][j].is_visible = false;
									points[n][i][j].is_highlighted = false;
									printf("Removing\n");
									//double_click = false;
									printf("Clicked on:\nX: %d\nY: %d\n", points[n][i][j].pos.x, points[n][i][j].pos.y);
								}
							}
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
				printf("Quiting\n\n\n");
				quit = true;
				kissquit = 1;
				kiss_clean(&objects);
				cleanup();
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(e.button.clicks == 1)
				{
					single_click = true;
					double_click = false;
					SDL_GetMouseState(&mouse_x, &mouse_y);
					mouse_moved = true;
					if ((e.type & SDL_BUTTON_LMASK) != 0)
					{
						left_mousedown = true;
					}
					if ((buttons & SDL_BUTTON_LMASK) != 0)
					{
					}
				} else if(e.button.clicks == 2)
				{
					double_click = true;
					printf("double click\n");
					single_click = false;
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

				single_click = false;
				//double_click = false;
				// }
				break;
			case SDL_FINGERUP:
				left_mousedown = false;
				mouse_moved = false;
				first_click_x = 0;
				first_click_y = 0;
				first_click = false;
				break;
			case SDL_WINDOWEVENT:
				if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				{
					paused = true;
					printf("Paused\n");
				} else
				if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				{
					paused = false;
					printf("UnPaused\n");
				}
				update_world(e);
				// Change graphbox coordinates to new window size
				init_box();
				break;
		}
		kiss_window_event(&kisswindow, &e, &kissdraw);
		kiss_entry_event(&entry, &e, &kissdraw);
		button_event(&button, &e, &kissdraw, &kissquit);
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
	if(get_world()->ASPECT_RATIO.x > get_world()->ASPECT_RATIO.y)
	{
		axes_horizontal_line_width = get_world()->render_distance.x * 3;
		axes_horizontal_line_height = 1*get_world()->world_dimensions.y;

		axes_vertical_line_width = 1*get_world()->world_dimensions.x;
		axes_vertical_line_height = get_world()->render_distance.y * 3;
	} else
	{
		axes_horizontal_line_width = get_world()->render_distance.x * 2;
		axes_horizontal_line_height = get_world()->world_dimensions.y * 1;

		axes_vertical_line_width = get_world()->world_dimensions.x * 1;
		axes_vertical_line_height = get_world()->render_distance.y * 2;
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
						points[n][i][j].pos.x = j;
						points[n][i][j].pos.y = i+1;

						points[n][i][j].rect.w = 15;
						points[n][i][j].rect.h = 15;
						break;
					case 1:
						points[n][i][j].pos.x = -j-1;
						points[n][i][j].pos.y = i+1;

						points[n][i][j].rect.w = 15;
						points[n][i][j].rect.h = 15;
						break;
					case 2:
						points[n][i][j].pos.x = -j-1;
						points[n][i][j].pos.y = -i;

						points[n][i][j].rect.w = 15;
						points[n][i][j].rect.h = 15;
						break;
					case 3:
						points[n][i][j].pos.x = j;
						points[n][i][j].pos.y = -i;

						points[n][i][j].rect.w = 15;
						points[n][i][j].rect.h = 15;
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
