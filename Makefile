CC = gcc

main: main.o
		$(CC) main.c -o chip8 -I include -L lib -lSDL2
