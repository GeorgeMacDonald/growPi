
#CFLAGS=-I /usr/include/cairo `pkg-config --cflags gtk+-wayland-3.0` 
#LDLIBS=`pkg-config --libs gtk+-wayland-3.0`

#CFLAGS=-I /usr/include/cairo `pkg-config --cflags gtk+-x11-3.0` 
#LDLIBS=`pkg-config --libs gtk+-x11-3.0`

CFLAGS=-I /usr/include/cairo `pkg-config --cflags gtk+-3.0` 
LDLIBS=`pkg-config --libs gtk+-3.0`


all: growPi

growPi: growPi.o game.o

growPi.o: growPi.c game.h

clean:
	rm *.o
