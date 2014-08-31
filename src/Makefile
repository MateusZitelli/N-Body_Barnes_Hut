CC = gcc
CFLAGS = -Wall -O3 -fopenmp -ffast-math -malign-double 
LIBS = -lglut -lGLU -lGL -lpng
DEPS = render.h config.h core.h 
OBJ = render.o core.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

all: $(OBJ)
	gcc $(CFLAGS) -o nbody.bin main.c $^ $(LIBS)

clean:
	rm -rf *.o *.bin *.csv
