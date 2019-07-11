
dbg: main.cc
	g++-8 -Wall main.cc -g -O0 -lstdc++fs -lcurses -std=c++17 -lgflags -lpthread -o main

all: main.cc
	g++-8 -Wall main.cc -lstdc++fs -lcurses -std=c++17 -lgflags -lpthread -march=native -mtune=native -O3 -o main

clean:
	rm -rf main

