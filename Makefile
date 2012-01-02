all:
	gcc main.c -o simula.bin -O3 `sdl-config --cflags` `sdl-config --libs` -lSDL_gfx -g -lglut  -lGLU -funroll-loops -ffast-math -malign-double -lpng
