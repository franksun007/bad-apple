
LIBS = -llibjpeg
LIBRARY_PATH = -Llibjpeg-turbo/build

all: main.cc
	g++ main.cc -lstdc++fs -lcurses -std=c++17 -march=native -O3 -o main 

