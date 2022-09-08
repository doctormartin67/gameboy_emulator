#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "emulator.h"

enum interrupt {
	INT_NONE,
	INT_VBLANK,
	INT_LCD_STAT,
	INT_TIMER = 4,
	INT_SERIAL = 8,
	INT_JOYPAD = 16,
};

void cpu_int_handler(Emulator *emu);

#endif
