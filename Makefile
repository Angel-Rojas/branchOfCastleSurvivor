#
# Makefile for asteroids game
#
# Enter one of the following
#
# make
# make all
# make asteroids
# make clean
#
CFLAGS = -I ./include
#LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm
LFLAGS = -lrt -lX11 -lGL

all: main

main: main.cpp timers.cpp log.cpp Common.h abdullahA.cpp abdullahA.h angelR.cpp angelR.h christy.cpp christy.h nygelA.cpp nygelA.h
	g++ $(CFLAGS) main.cpp log.cpp timers.cpp \
	libggfonts.a -Wall $(LFLAGS) -o main

clean:
	rm -f main
	rm -f *.o
