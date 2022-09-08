#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "emulator.h"

enum interrupt {
	INT_NONE,
	INT_VBLANK,
	INT_LCD_STAT,
	INT_TIMER,
	INT_SERIAL,
	INT_JOYPAD,
};

void cpu_int_handler(Emulator *emu);

#endif
