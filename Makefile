all:
	g++ main.c -o simula.bin -O3 `sdl-config --cflags` `sdl-config --libs` -lSDL_gfx -g -lglut  -lGLU -fopenmp -ffast-math -malign-double -lpng
	g++ viewer.c -o v.bin -O3 `sdl-config --cflags` `sdl-config --libs` -lSDL_gfx -g -lglut  -lGLU -fopenmp -ffast-math -malign-double -lpng
