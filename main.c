#include "main.h"

#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_events.h"
#include "include/SDL2/SDL_render.h"
#include "include/SDL2/SDL_video.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DELAY_RATE (15)

static ch8_t ch8; 

// Prototypes
void initialize();
bool load_rom(char *);
void emulate_cycle(bool *);
void draw(SDL_Renderer **);
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

  bool draw_flag = false;

  SDL_Event event;
  SDL_Renderer *renderer;
  SDL_Window *window;

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_CreateWindowAndRenderer(640, 320, 0, &window, &renderer);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  /*
  for (int i = (4*64); i < (6*64); ++i) {
    ch8.gfx[i] = 1; 
  }

  draw(&renderer);
  printf("Drew\n");
  */

  while (true) {
    // TODO: x button quits the program.
    if (SDL_PollEvent(&event)) {
      continue;
    }

    emulate_cycle(&draw_flag);
  
    if (draw_flag) {
      draw(&renderer);
      draw_flag = false;
    }
    // Set rate to a certain hz
    SDL_Delay(DELAY_RATE);
  }
 
  /*
  while (1) {
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
      break;
    }
  }
  */

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

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
 *  Loads rom of given rom name.
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

/*
 *  Completes one cycle (reads in one opcode) of the emulation.
 */

void emulate_cycle(bool *draw_flag) {

  ch8.opcode = ch8.memory[ch8.pc] << 8 | ch8.memory[ch8.pc + 1];

  ch8.pc += 2;

  /*
   * TODO:  Decode the opcode.
   */

  switch(ch8.opcode & 0xF000) {
    case 0x0000:
      switch(ch8.opcode & 0x000F) {
        case 0x0000: // 0x00E0: Clears the screen
          // TODO: See if this can be optimized.
          for (int i = 0;  i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); i++) {
            ch8.gfx[i] = 0;
          }
          break;

        case 0x000E: // 0x00EE: Returns from subroutine
          break;

        default:
          printf("Unknown opcode \"0x0000\": 0x%x\n", ch8.opcode);
      }
      break;

    case 0x1000: // 0x1NNN: Jump, setting the PC to NNN
      ch8.pc = ch8.opcode & 0x0FFF;
      break;
    
    case 0x6000: // 0x6XNN: Set the register VX to NN
      ch8.V[(ch8.opcode & 0x0F00) >> 8] = ch8.opcode & 0x00FF; 
      break;

    case 0x7000: // 0x7XNN: Add NN to register VX 
      ch8.V[(ch8.opcode & 0x0F00) >> 8] += ch8.opcode & 0x00FF; 
      break;

    case 0xA000: // 0xANNN: Set index register ch8.I to NNN
      ch8.I = ch8.opcode & 0x0FFF;
      break;

    case 0xD000: { // 0xDXYN: Draws sprites to screen.
      // TODO: Rewrite this code in my own way.
      unsigned short x = ch8.V[(ch8.opcode & 0x0F00) >> 8] % 64;
      unsigned short y = ch8.V[(ch8.opcode & 0x00F0) >> 4] % 32;
      unsigned short height = ch8.opcode & 0x000F;
      unsigned short pixel = 0;

      ch8.V[0xF] = 0;

      for (int i = 0; i < height; i++) {
        pixel = ch8.memory[ch8.I + i];
        
        for (int j = 0; j < 8; j++) {
          if ((pixel & (0x80 >> j)) != 0) {
            if (ch8.gfx[(x + j + ((y + i) * 64))] == 1) {
              ch8.V[0xF] = 1;
            }
            ch8.gfx[x + j + ((y + i) * 64)] ^= 1;
          }
        }
      }
      *draw_flag = true;
      break;
    }

    default:
      printf("Unknown opcode: 0x%x\n", ch8.opcode);
  }

  // Deincrement timers if necessary
  if (ch8.delay_timer > 0) {
    ch8.delay_timer--;
  }

  if (ch8.sound_timer > 0) {
    beep();
    ch8.sound_timer--;
  }
} /* emulateCycle() */

/*
 *  Draws to the surface.
 */

void draw(SDL_Renderer **renderer) {
  printf("Drawing...\n");

  SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0);
  SDL_RenderClear(*renderer);

  SDL_SetRenderDrawColor(*renderer, 255, 0, 0, 255);
  for (int x = 0; x < 640; x++) {
    for (int y = 0; y < 320; y++) {
      if (ch8.gfx[(x/10) + (y/10) * 64] == 1) {
        SDL_SetRenderDrawColor(*renderer, 255, 0, 0, 255);
        SDL_RenderDrawPoint(*renderer, x, y);
      }
    }
  } 
  SDL_RenderPresent(*renderer);
} /* draw() */

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
