CC = gcc

main: main.o
		$(CC) main.c -o main -I include -L lib -lSDL2
