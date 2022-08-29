#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "op.h"
#include "cartridge.h"

// https://gbdev.io/pandocs/CPU_Registers_and_Flags.html

struct registers {
	uint8_t a;
	uint8_t f;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint16_t sp;
	uint16_t pc;
}; 

typedef struct Cpu {
	struct registers regs;
	uint8_t opcode;
	Op op;
} Cpu;
void next_op(Cpu *cpu, const Cartridge *cart);
void cpu_print(const Cpu *cpu, const Cartridge *cart);

#endif
