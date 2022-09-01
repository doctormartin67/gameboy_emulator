#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "op.h"

// https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

static const Op ops[0x100] = {
	[0x00] = {.kind = NOP},
	[0x01] = {.kind = LD_RR_IMM16, .reg1 = REG_BC},
	[0x02] = {.kind = LD_ARR_R, .reg1 = REG_BC, .reg2 = REG_A},
	[0x05] = {.kind = DEC_R, .reg1 = REG_B},
	[0x06] = {.kind = LD_R_IMM8, .reg1 = REG_B},
	[0x0a] = {.kind = LD_R_ARR, .reg1 = REG_A, .reg2 = REG_BC},
	[0x0e] = {.kind = LD_R_IMM8, .reg1 = REG_C},

	[0x11] = {.kind = LD_RR_IMM16, .reg1 = REG_DE},
	[0x12] = {.kind = LD_ARR_R, .reg1 = REG_DE, .reg2 = REG_A},
	[0x16] = {.kind = LD_R_IMM8, .reg1 = REG_D},
	[0x1a] = {.kind = LD_R_ARR, .reg1 = REG_A, .reg2 = REG_DE},
	[0x1e] = {.kind = LD_R_IMM8, .reg1 = REG_E},

	[0x21] = {.kind = LD_RR_IMM16, .reg1 = REG_HL},
	[0x22] = {.kind = LD_ARRI_R, .reg1 = REG_HL, .reg2 = REG_A},
	[0x26] = {.kind = LD_R_IMM8, .reg1 = REG_H},
	[0x2a] = {.kind = LD_R_ARRI, .reg1 = REG_A, .reg2 = REG_HL},
	[0x2e] = {.kind = LD_R_IMM8, .reg1 = REG_L},

	[0x31] = {.kind = LD_RR_IMM16, .reg1 = REG_SP},
	[0x32] = {.kind = LD_ARRD_R, .reg1 = REG_HL, .reg2 = REG_A},
	[0x36] = {.kind = LD_ARR_IMM8, .reg1 = REG_HL},
	[0x3a] = {.kind = LD_R_ARRD, .reg1 = REG_A, .reg2 = REG_HL},
	[0x3e] = {.kind = LD_R_IMM8, .reg1 = REG_A},

	[0x40] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_B},
	[0x41] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_C},
	[0x42] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_D},
	[0x43] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_E},
	[0x44] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_H},
	[0x45] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_L},
	[0x46] = {.kind = LD_R_ARR, .reg1 = REG_B, .reg2 = REG_HL},
	[0x47] = {.kind = LD_R_R, .reg1 = REG_B, .reg2 = REG_A},
	[0x48] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_B},
	[0x49] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_C},
	[0x4a] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_D},
	[0x4b] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_E},
	[0x4c] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_H},
	[0x4d] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_L},
	[0x4e] = {.kind = LD_R_ARR, .reg1 = REG_C, .reg2 = REG_HL},
	[0x4f] = {.kind = LD_R_R, .reg1 = REG_C, .reg2 = REG_A},

	[0x50] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_B},
	[0x51] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_C},
	[0x52] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_D},
	[0x53] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_E},
	[0x54] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_H},
	[0x55] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_L},
	[0x56] = {.kind = LD_R_ARR, .reg1 = REG_D, .reg2 = REG_HL},
	[0x57] = {.kind = LD_R_R, .reg1 = REG_D, .reg2 = REG_A},
	[0x58] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_B},
	[0x59] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_C},
	[0x5a] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_D},
	[0x5b] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_E},
	[0x5c] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_H},
	[0x5d] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_L},
	[0x5e] = {.kind = LD_R_ARR, .reg1 = REG_E, .reg2 = REG_HL},
	[0x5f] = {.kind = LD_R_R, .reg1 = REG_E, .reg2 = REG_A},

	[0x60] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_B},
	[0x61] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_C},
	[0x62] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_D},
	[0x63] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_E},
	[0x64] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_H},
	[0x65] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_L},
	[0x66] = {.kind = LD_R_ARR, .reg1 = REG_H, .reg2 = REG_HL},
	[0x67] = {.kind = LD_R_R, .reg1 = REG_H, .reg2 = REG_A},
	[0x68] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_B},
	[0x69] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_C},
	[0x6a] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_D},
	[0x6b] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_E},
	[0x6c] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_H},
	[0x6d] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_L},
	[0x6e] = {.kind = LD_R_ARR, .reg1 = REG_L, .reg2 = REG_HL},
	[0x6f] = {.kind = LD_R_R, .reg1 = REG_L, .reg2 = REG_A},

	[0x70] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_B},
	[0x71] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_C},
	[0x72] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_D},
	[0x73] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_E},
	[0x74] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_H},
	[0x75] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_L},
	[0xf6] = {.kind = HALT},
	[0x77] = {.kind = LD_ARR_R, .reg1 = REG_HL, .reg2 = REG_A},
	[0x78] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_B},
	[0x79] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_C},
	[0x7a] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_D},
	[0x7b] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_E},
	[0x7c] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_H},
	[0x7d] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_L},
	[0x7e] = {.kind = LD_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0x7f] = {.kind = LD_R_R, .reg1 = REG_A, .reg2 = REG_A},

	[0xaf] = {.kind = XOR_R, .reg1 = REG_A}, 

	[0xc3] = {.kind = JP_IMM16},

	[0xe0] = {.kind = LDH_AIMM8_R, .reg2 = REG_A},
	[0xe2] = {.kind = LD_AR_R, .reg1 = REG_C, .reg2 = REG_A},
	[0xea] = {.kind = LD_AIMM16_R, .reg2 = REG_A},

	[0xf0] = {.kind = LDH_R_AIMM8, .reg1 = REG_A},
	[0xf2] = {.kind = LD_R_AR, .reg1 = REG_A, .reg2 = REG_C},
	[0xf3] = {.kind = DI},
	[0xf8] = {.kind = LD_RR_RR_IMM8, .reg1 = REG_SP, .reg2 = REG_HL},
	[0xf9] = {.kind = LD_RR_RR, .reg1 = REG_SP, .reg2 = REG_HL},
	[0xfa] = {.kind = LD_R_AIMM16, .reg1 = REG_A},
};

static const char *const op_names[0x100] = {
	[0x00] = "NOP",
	[0x01] = "LD_RR_IMM16",
	[0x02] = "LD_ARR_R",
	[0x05] = "DEC_R",
	[0x06] = "LD_R_IMM8",
	[0x0a] = "LD_R_ARR",
	[0x0e] = "LD_R_IMM8",

	[0x11] = "LD_RR_IMM16",
	[0x12] = "LD_ARR_R",
	[0x16] = "LD_R_IMM8",
	[0x1a] = "LD_R_ARR",
	[0x1e] = "LD_R_IMM8",

	[0x21] = "LD_RR_IMM16",
	[0x22] = "LD_ARRI_R",
	[0x26] = "LD_R_IMM8",
	[0x2a] = "LD_R_ARRI",
	[0x2e] = "LD_R_IMM8",

	[0x31] = "LD_RR_IMM16",
	[0x32] = "LD_ARRD_R",
	[0x36] = "LD_ARR_IMM8",
	[0x3a] = "LD_R_ARRD",
	[0x3e] = "LD_R_IMM8",

	[0x40] = "LD_R_R",
	[0x41] = "LD_R_R",
	[0x42] = "LD_R_R",
	[0x43] = "LD_R_R",
	[0x44] = "LD_R_R",
	[0x45] = "LD_R_R",
	[0x46] = "LD_R_ARR",
	[0x47] = "LD_R_R",
	[0x48] = "LD_R_R",
	[0x49] = "LD_R_R",
	[0x4a] = "LD_R_R",
	[0x4b] = "LD_R_R",
	[0x4c] = "LD_R_R",
	[0x4d] = "LD_R_R",
	[0x4e] = "LD_R_ARR",
	[0x4f] = "LD_R_R",

	[0x50] = "LD_R_R",
	[0x51] = "LD_R_R",
	[0x52] = "LD_R_R",
	[0x53] = "LD_R_R",
	[0x54] = "LD_R_R",
	[0x55] = "LD_R_R",
	[0x56] = "LD_R_ARR",
	[0x57] = "LD_R_R",
	[0x58] = "LD_R_R",
	[0x59] = "LD_R_R",
	[0x5a] = "LD_R_R",
	[0x5b] = "LD_R_R",
	[0x5c] = "LD_R_R",
	[0x5d] = "LD_R_R",
	[0x5e] = "LD_R_ARR",
	[0x5f] = "LD_R_R",

	[0x60] = "LD_R_R",
	[0x61] = "LD_R_R",
	[0x62] = "LD_R_R",
	[0x63] = "LD_R_R",
	[0x64] = "LD_R_R",
	[0x65] = "LD_R_R",
	[0x66] = "LD_R_ARR",
	[0x67] = "LD_R_R",
	[0x68] = "LD_R_R",
	[0x69] = "LD_R_R",
	[0x6a] = "LD_R_R",
	[0x6b] = "LD_R_R",
	[0x6c] = "LD_R_R",
	[0x6d] = "LD_R_R",
	[0x6e] = "LD_R_ARR",
	[0x6f] = "LD_R_R",

	[0x70] = "LD_ARR_R",
	[0x71] = "LD_ARR_R",
	[0x72] = "LD_ARR_R",
	[0x73] = "LD_ARR_R",
	[0x74] = "LD_ARR_R",
	[0x75] = "LD_ARR_R",
	[0xf6] = "HALT",
	[0x77] = "LD_ARR_R",
	[0x78] = "LD_R_R",
	[0x79] = "LD_R_R",
	[0x7a] = "LD_R_R",
	[0x7b] = "LD_R_R",
	[0x7c] = "LD_R_R",
	[0x7d] = "LD_R_R",
	[0x7e] = "LD_R_ARR",
	[0x7f] = "LD_R_R",

	[0xaf] = "XOR_R",

	[0xc3] = "JP_IMM16",

	[0xe0] = "LDH_AIMM8_R",
	[0xe2] = "LD_AR_R",
	[0xea] = "LD_AIMM16_R",

	[0xf0] = "LDH_R_AIMM8",
	[0xf2] = "LD_R_AR",
	[0xf3] = "DI",
	[0xf8] = "LD_RR_RR_IMM8",
	[0xf9] = "LD_RR_RR",
	[0xfa] = "LD_R_AIMM16",
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
