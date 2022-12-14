#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "op.h"
#include "interrupts.h"

// https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
#define FLAG_Z BIT(cpu->regs.f, FLAG_Z_BIT)
#define FLAG_N BIT(cpu->regs.f, FLAG_N_BIT)
#define FLAG_H BIT(cpu->regs.f, FLAG_H_BIT)
#define FLAG_C BIT(cpu->regs.f, FLAG_C_BIT)

#define Z_FLAG(a, b, op) !(((a) op (b)) & 0xff)
#define N_FLAG(a, b, op) BIT((a) op (b), sizeof((a) op (b)) - 1)
#define H_FLAG(a, b, op) (uint64_t)(((a) & 0xf) op ((b) & 0xf)) > 0xf
#define C_FLAG(a, b, op) (uint64_t)(((a) & 0xff) op ((b) & 0xff)) > 0xff
#define H_FLAG16(a, b, op) (uint64_t)(((a) & 0xfff) op ((b) & 0xfff)) > 0xfff
#define C_FLAG16(a, b, op) (uint64_t)(((a) & 0xffff) op ((b) & 0xffff)) > 0xffff

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

// https://gbdev.io/pandocs/Interrupts.html
typedef struct Cpu {
	struct registers regs;
	unsigned ime_flag; // interrupt master enable flag
	unsigned delay_interrupt; // after EI there is a delay of 1 cycle
	unsigned halted;
	uint8_t ie_reg; // interrupt enable
	uint8_t if_reg; // interrupt flag
	uint8_t opcode;
	Op op;
} Cpu;
uint8_t cpu_ie_reg_read(const Cpu *cpu);
void cpu_ie_reg_write(Cpu *cpu, uint8_t data);
uint8_t cpu_if_reg_read(const Cpu *cpu);
void cpu_request_interrupt(Cpu *cpu, enum interrupt it);
Cpu *cpu_init(void);
void free_cpu(Cpu *cpu);

#endif
