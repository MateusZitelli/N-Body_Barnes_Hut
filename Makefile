SRCDIR=src
CC = gcc
CFLAGS = -Wall -O3 -fopenmp -ffast-math -malign-double 
LIBS = -lglut -lGLU -lGL -lpng
_DEPS = render.h config.h core.h 
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = render.o core.o
OBJ = $(patsubst %,$(SRCDIR)/%,$(_OBJ))

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

all: $(OBJ)
	gcc $(CFLAGS) -o nbody.bin $(SRCDIR)/main.c $^ $(LIBS)

clean:
	rm -rf $(SRCDIR)/*.o *.bin *.csv
