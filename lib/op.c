#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "op.h"

static const Op ops[0x100] = {
	[0x00] = {.kind = NOP},
	[0x05] = {.kind = DEC_R, .reg1 = REG_B},
	[0x0e] = {.kind = LD_R_IMM8, .reg1 = REG_C},
	[0xaf] = {.kind = XOR_R, .reg1 = REG_A}, 
	[0xc3] = {.kind = JP_IMM16},
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
