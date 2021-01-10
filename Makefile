all:
	g++ -O3 -o chip8 CPU.cpp app.cpp -lncurses

dasm:
	g++ -O3 -o chip8dasm disassembler.cpp

run:
	./chip8