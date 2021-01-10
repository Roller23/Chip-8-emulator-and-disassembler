all:
	g++ -O3 -o chip8 CPU.cpp app.cpp -lncurses

run:
	./chip8