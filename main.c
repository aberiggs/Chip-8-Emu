#include "main.h"

#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_events.h"
#include "include/SDL2/SDL_render.h"
#include "include/SDL2/SDL_video.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define DELAY_RATE (1000/60) // 1000/60 is 60 FPS
#define DISPLAY_SCALE (20)

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

  if(!load_rom("pong.rom")) {
    printf("Failed to load rom! Exiting...\n");
    return 0;
  }
  printf("Rom Loaded...\n");

  bool draw_flag = false;

  SDL_Event event;
  SDL_Renderer *renderer;
  SDL_Window *window;

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_CreateWindowAndRenderer(DISPLAY_WIDTH * DISPLAY_SCALE,
                              DISPLAY_HEIGHT * DISPLAY_SCALE, 0, &window,
                              &renderer);
  SDL_RenderClear(renderer);

  while (true) {
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
      break;
    }

    emulate_cycle(&draw_flag);
  
    if (draw_flag) {
      draw(&renderer);
      draw_flag = false;
    }
    // Set rate to a certain hz
    SDL_Delay(DELAY_RATE);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

void initialize() {
  ch8 = (ch8_t) {0}; // Set everything to zero.

  ch8.pc = 0x200; // Program counter starts at where the rom is to be loaded.
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

  ch8.opcode = ch8.memory[ch8.pc] << 8 | ch8.memory[ch8.pc + 1]; // Get opcode

  ch8.pc += 2; // Move to the next instruction.

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
          ch8.sp--;
          ch8.pc = ch8.stack[ch8.sp];
          break;

        default:
          printf("Unknown opcode \"0x0000\": 0x%x\n", ch8.opcode);
      }
      break;

    case 0x1000: // 0x1NNN: Jump, setting the PC to NNN
      ch8.pc = ch8.opcode & 0x0FFF;
      break;

    case 0x2000: // 0x2NNN: Push PC to stack and then set PC to NNN
      ch8.stack[ch8.sp] = ch8.pc;
      ch8.sp++;
      break;

    case 0x3000: // 0x3XNN: Skip an instruction if VX is equal to NN
      if (ch8.V[(ch8.opcode & 0x0F00) >> 8] == (ch8.opcode & 0x00FF)) {
        ch8.pc += 2;
      }
      break;

    case 0x4000: // 0x4XNN Skip an instruction if VX is not equal to NN
      if (ch8.V[(ch8.opcode & 0x0F00) >> 8] != (ch8.opcode & 0x00FF)) {
        ch8.pc += 2;
      }
      break;

    case 0x5000: // 0x5XY0: Skips an instruction if VX and VY are equal
      if (ch8.V[(ch8.opcode & 0x0F00) >> 8] ==
          ch8.V[(ch8.opcode & 0x00F0) >> 4]) {
        ch8.pc += 2;
      }
      break;

    case 0x6000: // 0x6XNN: Set the register VX to NN
      ch8.V[(ch8.opcode & 0x0F00) >> 8] = ch8.opcode & 0x00FF; 
      break;

    case 0x7000: // 0x7XNN: Add NN to register VX 
      ch8.V[(ch8.opcode & 0x0F00) >> 8] += ch8.opcode & 0x00FF; 
      break;

    case 0x8000:
      switch(ch8.opcode & 0x000F) {
        case 0x0000: // 0x8XY0: Set VX to VY
          ch8.V[(ch8.opcode & 0x0F00) >> 8] =
            ch8.V[(ch8.opcode & 0x00F0) >> 4];
          break;

        case 0x0001: // 0x8XY1: Set VX to binary or of VX and VY
          ch8.V[(ch8.opcode & 0x0F00) >> 8] |=
            ch8.V[(ch8.opcode & 0x00F0) >> 4];
          break;

        case 0x0002: // 0x8XY2: Set VX to binary and of VX and VY
          ch8.V[(ch8.opcode & 0x0F00) >> 8] &=
            ch8.V[(ch8.opcode & 0x00F0) >> 4];
          break;

        case 0x0003: // 0x8XY3: Set VX to binary xor of VX and VY
          ch8.V[(ch8.opcode & 0x0F00) >> 8] ^=
            ch8.V[(ch8.opcode & 0x00F0) >> 4];
          break;

        case 0x0004: // 0x8XY4: Set VX to VX plus VY
          ch8.V[0xF] = 0;
          if ((ch8.V[(ch8.opcode & 0x0F00) >> 8] +
              ch8.V[(ch8.opcode & 0x00F0) >> 4]) > 255) {
            ch8.V[0xF] = 1;
          }
          ch8.V[(ch8.opcode & 0x0F00) >> 8] +=
            ch8.V[(ch8.opcode & 0x00F0) >> 4];
          break;

        case 0x0005: // 0x8XY5: VX is VX - VY
          ch8.V[(ch8.opcode & 0x0F00) >> 8] -=
            ch8.V[(ch8.opcode & 0x00F0) >> 4];

          ch8.V[0xF] = 1;

          if (ch8.V[(ch8.opcode & 0x0F00) >> 8] <
              ch8.V[(ch8.opcode & 0x00F0) >> 4]) {
            ch8.V[0xF] = 0;
          }
          break;

        case 0x0006: // 0x8XY6: Shift VX one bit to the right
          ch8.V[0xF] = 0;
          if (ch8.V[(ch8.opcode & 0x0F00) >> 8] & 0x1) {
            ch8.V[0xF] = 1;
          }
          ch8.V[(ch8.opcode & 0x0F00) >> 8] =
            (ch8.V[(ch8.opcode & 0x0F00) >> 8] >> 1);
          break;

        case 0x0007: // 0x8XY5: VX is VY - VX
          ch8.V[(ch8.opcode & 0x00F0) >> 4] -=
            ch8.V[(ch8.opcode & 0x0F00) >> 8];

          ch8.V[0xF] = 1;

          if (ch8.V[(ch8.opcode & 0x0F00) >> 8] >
              ch8.V[(ch8.opcode & 0x00F0) >> 4]) {
            ch8.V[0xF] = 0;
          }
          break;

        case 0x000E: /// 0x8XY6: Shift VX 1 bit left
          ch8.V[0xF] = 0;
          // TODO: Check this for bugs :)
          if (ch8.V[(ch8.opcode & 0x0F00) >> 8] >> 3) {
            ch8.V[0xF] = 1;
          }
          ch8.V[(ch8.opcode & 0x0F00) >> 8] =
            (ch8.V[(ch8.opcode & 0x0F00) >> 8] << 1);
          break;

        default:
          printf("Unknown opcode \"0x8000\": 0x%x\n", ch8.opcode);
      }

    case 0x9000: // 0x9XY0: Skips an instruction if VX and VY are not equal
      if (ch8.V[(ch8.opcode & 0x0F00) >> 8] !=
          ch8.V[(ch8.opcode & 0x00F0) >> 4]) {
        ch8.pc += 2;
      }
      break;

    case 0xA000: // 0xANNN: Set index register ch8.I to NNN
      ch8.I = ch8.opcode & 0x0FFF;
      break;

    case 0xB000: // 0xBNNN: Jump with offset
      ch8.pc = ch8.memory[(ch8.opcode & 0x0FFF) +
               (ch8.V[(ch8.opcode & 0x0F00) >> 8])];
      break;

    case 0xC000: // 0xCXNN: Generates random number
      srand(time(0));
      ch8.V[(ch8.opcode & 0x0F00) >> 8] = (rand() & (ch8.opcode & 0x00FF));
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

    case 0xF000: { //TODO: Label what each opcode does.
      switch (ch8.opcode & 0x00FF) {
        case 0x0007:
          ch8.V[(ch8.opcode & 0x0F00) >> 8] = ch8.delay_timer;
          break;

        case 0x0015:
          ch8.delay_timer = ch8.V[(ch8.opcode & 0x0F00) >> 8];
          break;

        case 0x0018:
          ch8.sound_timer = ch8.V[(ch8.opcode & 0x0F00) >> 8];
          break;

        case 0x001E:
          ch8.I += ch8.V[(ch8.opcode & 0x0F00) >> 8];
          break;

        case 0x0033:
        {
          unsigned char num = ch8.V[(ch8.opcode & 0x0F00) >> 8];
          ch8.memory[ch8.I] = num / 100; // Get hundreds place
          ch8.memory[ch8.I + 1] = (num % 100) / 10; // Get tens place
          ch8.memory[ch8.I + 2] = (num % 10); // Get ones place
          break;
        }
      }
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

  // Clear the screen
  SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0);
  SDL_RenderClear(*renderer);

  // Draw the graphics
  SDL_SetRenderDrawColor(*renderer, 255, 0, 255, 255);
  for (int x = 0; x < DISPLAY_WIDTH * DISPLAY_SCALE; x++) {
    for (int y = 0; y < DISPLAY_HEIGHT * DISPLAY_SCALE; y++) {
      if (ch8.gfx[(x/DISPLAY_SCALE) + (y/DISPLAY_SCALE) * 64] == 1) {
        SDL_RenderDrawPoint(*renderer, x, y);
      }
    }
  } 
  SDL_RenderPresent(*renderer); // Display the changes
} /* draw() */

/*
 *  Emits beeping noise on the system.
 */

void beep() {
  printf("Beep!");
} /* beep() */
