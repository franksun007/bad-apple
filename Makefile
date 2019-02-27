all: main.cc
	g++-8 -Wall main.cc -lstdc++fs -lcurses -std=c++17 -lgflags -lpthread -march=native -O3 -o main

clean:
	rm -rf main

