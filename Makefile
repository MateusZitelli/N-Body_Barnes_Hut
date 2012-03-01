all:
	gcc main.c -o simula.bin -O3 -g -lglut  -lGLU -fopenmp -ffast-math -malign-double -lpng
	gcc viewer.c -o v.bin -O3  -g -lglut  -lGLU -fopenmp -ffast-math -malign-double -lpng
