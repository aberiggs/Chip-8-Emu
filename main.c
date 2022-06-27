#include "main.h"

#include "include/SDL2/SDL.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {
  printf("Welcome to Sprocket's Chip-8 Emulator...\n");

  int scaling = 5;
  int window_width = scaling * DISPLAY_WIDTH;
  int window_height = scaling * DISPLAY_HEIGHT;

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("Error initializing SDL %s\n", SDL_GetError());
  }


  SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                   window_width, window_height,
                   SDL_WINDOW_RESIZABLE);

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

  return 0;
}
