all:
	cd src; gcc font_handler.c world.c main.c -lSDL2 -lSDL2_image -lSDL2_ttf

clean:
	rm a.out
