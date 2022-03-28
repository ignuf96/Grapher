all:
	cd src; gcc grapher_main.c font_handler.c platform.c -lSDL2 -lSDL2_image -lSDL2_ttf

clean:
	rm a.out
