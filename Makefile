all:
	g++ -O3 -o chip8 CPU.cpp app.cpp -lncurses -std=c++17

dasm:
	g++ -O3 -o chip8dasm disassembler.cpp

run:
	./chip8