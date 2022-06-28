#include "main.h"

#include "include/SDL2/SDL.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


ch8_t ch8; 

void initialize();

int main() {
  printf("Welcome to Sprocket's Chip-8 Emulator...\n");

  initialize();
  printf("Emulator initialized!\n");

  printf("Random memory char: %x\n", ch8.memory[6]);


  /*
  int scaling = 5;
  int window_width = scaling * DISPLAY_WIDTH;
  int window_height = scaling * DISPLAY_HEIGHT;

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("Error initializing SDL %s\n", SDL_GetError());
  }


  SDL_Window* win = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, 
                                     SDL_WINDOWPOS_CENTERED, window_width,
                                     window_height, SDL_WINDOW_RESIZABLE);

  SDL_Event e;

  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }
  }

  SDL_Quit();
  */
  return 0;
}

void initialize() {
  ch8 = {0};

  ch8.pc = 0x200;
  ch8.opcode = 0;
  ch8.I = 0;
  ch8.sp = 0;

  // Fontset loading
  for (int i = 0; i < 80; i++) { // TODO: multigesture has pre-increment but seems like it should be post ??? 
    ch8.memory[i] = chip8_fontset[i];
  }
}
