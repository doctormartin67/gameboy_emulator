#ifndef EMULATOR_H
#define EMULATOR_H

#include "cpu.h"
#include "cartridge.h"
#include "timer.h"

typedef struct Emulator {
	unsigned running;
	unsigned playing;
	uint64_t ticks;
	Cpu *cpu;
	Cartridge *cart;
	Timer *timer;
} Emulator;

int emu_main(int argc, char *argv[]);
Emulator *emu_init(Cpu *cpu, Cartridge *cart, Timer *timer);
void emu_ticks(Emulator *emu, unsigned ticks);
void emu_kill(Emulator *emu);
void cpu_print(const Emulator *emu);
void next_op(Emulator *emu);
void cpu_int_handler(Emulator *emu);

#endif
