all:
	cd src; gcc kiss_general.c kiss_draw.c kiss_posix.c kiss_widgets.c parser.c font_handler.c world.c main.c -lSDL2 -lSDL2_image -lSDL2_ttf

clean:
	rm a.out
