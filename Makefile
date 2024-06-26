C = gcc
CFLAGS = -std=c99 -g -pedantic -Wall
LIBS = -lm `pkg-config --cflags --libs sdl2`

CFILES = main.c

vgarogue-sdl: $(CFILES) clean
	$(CC) $(CFLAGS) -o vgarogue-sdl $(CFILES) $(LIBS)

clean:
		rm -vf vgarogue-sdl
