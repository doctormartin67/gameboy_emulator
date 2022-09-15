#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include "emulator.h"

// https://gbdev.io/pandocs/Tile_Data.html

#define SCALE 2
#define PIXELS 8

#define WINDOW_WIDTH 32 * PIXELS * SCALE
#define WINDOW_HEIGHT 48 * PIXELS * SCALE
#define WINDOW_FLAGS 0 // no flags at the moment

#define RENDERER_FLAGS 0 // no flags at the moment

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0XFF000000
#define GMASK 0X00FF0000
#define BMASK 0X0000FF00
#define AMASK 0X000000FF
#else
#define RMASK 0X000000FF
#define GMASK 0X0000FF00
#define BMASK 0X00FF0000
#define AMASK 0XFF000000
#endif

#define TILE_ADDR VRAM_ADDR
#define TILE_END_ADDR 0x9800

void ui_init(void);
void ui_handle_events(Emulator *emu);
void ui_update(const Emulator *emu);

#endif
