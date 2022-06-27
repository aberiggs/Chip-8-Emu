#ifndef MAIN_H
#define MAIN_H

#define RAM_SIZE (4069)

#define DISPLAY_WIDTH (64)
#define DISPLAY_HEIGHT (32)

typedef struct chip_8 {
  unsigned short opcode;
  unsigned char memory[RAM_SIZE];
  unsigned char V[16];
  unsigned short I;
  unsigned short pc;
  unsigned char gfx[DISPLAY_WIDTH * DISPLAY_HEIGHT];
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned short stack[16];
  unsigned short sp;
  unsigned char key[16];
} ch8_t;

#endif
