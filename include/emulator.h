#ifndef EMULATOR_H
#define EMULATOR_H

#include "cpu.h"
#include "cartridge.h"

typedef struct Emulator {
	unsigned running;
	unsigned playing;
	Cpu *cpu;
	Cartridge *cart;
} Emulator;

int emu_main(int argc, char *argv[]);
Emulator *emu_init(Cpu *cpu, Cartridge *cart);
void emu_kill(Emulator *emu);

#endif
