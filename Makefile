all:
	cd src; gcc main.c font_handler.c platform.c -lSDL2 -lSDL2_image -lSDL2_ttf

clean:
	rm a.out
