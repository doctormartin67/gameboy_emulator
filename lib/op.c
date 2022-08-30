#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "op.h"

static const Op ops[0x100] = {
	[0x00] = {.kind = NOP},
	[0x05] = {.kind = DEC_R, .reg1 = REG_B},
	[0x06] = {.kind = LD_R_IMM8, .reg1 = REG_B},
	[0x0e] = {.kind = LD_R_IMM8, .reg1 = REG_C},
	[0x21] = {.kind = LD_RR_IMM16, .reg1 = REG_HL},
	[0x32] = {.kind = LD_ARRI_R, .reg1 = REG_HL, .reg2 = REG_A},
	[0xaf] = {.kind = XOR_R, .reg1 = REG_A}, 
	[0xc3] = {.kind = JP_IMM16},
	[0xf3] = {.kind = DI},
};

static const char *const op_names[0x100] = {
	[0x00] = "NOP",
	[0x05] = "DEC",
	[0x06] = "LD",
	[0x0e] = "LD",
	[0x21] = "LD",
	[0x32] = "LD",
	[0xaf] = "XOR",
	[0xc3] = "JP",
	[0xf3] = "DI",
};

Op get_op_from_opcode(uint8_t opcode)
{
	Op op = ops[opcode];
	if (NONE == op.kind) {
		printf("Opcode '0x%02x' not implemented.\n", opcode);
		exit(1);
	}
	return op;
}

const char *op_name(uint8_t opcode)
{
	const char *name = op_names[opcode];
	return name;
}
