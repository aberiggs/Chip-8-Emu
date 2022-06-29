#include "main.h"

#include "include/SDL2/SDL.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DELAY_RATE (15)

static ch8_t ch8; 

// Prototypes
void initialize();
bool load_rom(char *);
void emulate_cycle();
void beep();

int main() {
  printf("Welcome to Sprocket's Chip-8 Emulator...\n");

  initialize();
  printf("Emulator initialized!\n");

  if(!load_rom("IBM.ch8")) {
    printf("Failed to load rom! Exiting...\n");
    return 0;
  }
  printf("Rom Loaded...\n");
  
  int check_loc = 0x201;
  printf("Hex at char location 0x%x: 0x%x\n", check_loc, ch8.memory[check_loc]);
  
  return 0;
}

void initialize() {
  ch8 = (ch8_t) {0}; // Set everything to zero.

  ch8.pc = 0x200;
  ch8.opcode = 0;
  ch8.I = 0;
  ch8.sp = 0;

  // Fontset loading
  for (int i = 0; i < 80; i++) { // TODO: multigesture has pre-increment but seems like it should be post ??? 
    ch8.memory[i] = chip8_fontset[i];
  }
} /* initialize() */

/*
 *  Completes one cycle (reads in one opcode) of the emulation.
 */

bool load_rom(char *rom_name) {
  
  FILE *rom = 0;
  rom = fopen(rom_name, "rb");

  if (!rom) {
    return false;
  }

  fread(ch8.memory + 0x200, RAM_SIZE - 0x200, 1, rom);

  return true;
} /* load_rom() */

void emulate_cycle() {

  ch8.opcode = ch8.memory[ch8.pc] << 8 | ch8.memory[ch8.pc + 1];

  /*
   * TODO:  Decode the opcode.
   */

  // Deincrement timers if necessary
  if (ch8.delay_timer > 0) {
    ch8.delay_timer--;
  }

  if (ch8.sound_timer > 0) {
    beep();
    ch8.sound_timer--;
  }

  // Set rate to a certain hz
  SDL_Delay(DELAY_RATE);

} /* emulateCycle() */


/*
 *  Emits beeping noise on the system.
 */

void beep() {
  printf("Beep!");
} /* beep() */

void display_setup() { 
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
}
