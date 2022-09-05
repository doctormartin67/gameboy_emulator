#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include "emulator.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define WINDOW_FLAGS 0 // no flags at the moment

#define RENDERER_FLAGS 0 // no flags at the moment

void ui_init(void);
void ui_handle_events(Emulator *emu);

#endif
