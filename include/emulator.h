#ifndef EMULATOR_H
#define EMULATOR_H

#include "cpu.h"
#include "cartridge.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "sound.h"

enum {
	TICKS_PER_CYCLE = 4,
};

typedef struct Emulator {
	unsigned running;
	unsigned playing;
	uint64_t ticks;
	Cpu *cpu;
	Cartridge *cart;
	Timer *timer;
	Ppu *ppu;
	struct joypad joypad;
	struct sound sound;
} Emulator;

int emu_main(int argc, char *argv[]);
Emulator *emu_init(Cpu *cpu, Cartridge *cart, Timer *timer, Ppu *ppu);
void emu_ticks(Emulator *emu, unsigned ticks);
void emu_kill(Emulator *emu);
void print_status(Emulator *emu);
void next_op(Emulator *emu);
void cpu_int_handler(Emulator *emu);
void dma_tick(Emulator *emu);
void ppu_tick(Emulator *emu);

#endif
