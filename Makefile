CPPFLAGS = -std=c++11
SRC = light.cpp
OBJ = $(SRC:.cpp=.o)
LIBS += -l SDL2 -l SDL2_image
all : light

light: ${OBJ}
	g++ -o $@ ${OBJ} ${LIBS}

run:
	./light

clean:
	rm -rf light
	rm -rf *.o
