# CHIP-8 emulator and disassembler

A fully working CHIP-8 emulator and disassembler. It uses ncurses to render graphics and gather user input as well as a few other posix libraries, therefore it will only compile on OS X, most Linux distributions, and WSL.
Since it uses ncurses for input, playing games is a bit wonky. It is recommended for learning purposes only, there are better CHIP-8 emulators to play games out there.

app.cpp provides a simple interface that allows you to choose a CHIP-8 program to run.
In-game controls are mapped to 1, 2, 3, 4, q, w, e, r, a, s, d, f, z, x, c, v and the keys' use varies by game.

The disassembler takes CHIP-8 game file path as input and outputs assembly-like code in a separate file.
Since there's no official CHIP-8 syntax, the instruction set from http://devernay.free.fr/hacks/chip8/chip8def.htm was used as a reference. 
The tool is useful to examine a game's low-level logic in a more human readable way.

To learn more about CHIP-8 visit the wiki page https://en.wikipedia.org/wiki/CHIP-8

Neither the emulator nor disassembler implement the Super CHIP-8 opcodes! Additionally, both of them do not recognize the 0NNN opcode as it's not used in most games.
