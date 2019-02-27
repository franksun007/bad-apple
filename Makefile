
LIBS = -llibjpeg
LIBRARY_PATH = -Llibjpeg-turbo/build

all: main.cc
	/usr/local/Cellar/gcc/8.3.0/bin/g++-8 main.cc -lstdc++fs -lcurses -std=c++17 -march=native -O3 -o main 

