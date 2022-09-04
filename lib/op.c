#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "op.h"

// https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

static const Op ops[0x100] = {
	[0x00] = {.kind = NOP},
	[0x01] = {.kind = LD_RR_IMM16, .reg1 = REG_BC},
	[0x02] = {.kind = LD_ARR_R, .reg1 = REG_BC, .reg2 = REG_A},
	[0x03] = {.kind = INC_RR, .reg1 = REG_BC},
	[0x05] = {.kind = DEC_R, .reg1 = REG_B},
	[0x06] = {.kind = LD_R_IMM8, .reg1 = REG_B},
	[0x07] = {.kind = RLCA, .reg1 = REG_A},
	[0x09] = {.kind = ADD_RR_RR, .reg1 = REG_HL, .reg2 = REG_BC},
	[0x0a] = {.kind = LD_R_ARR, .reg1 = REG_A, .reg2 = REG_BC},
	[0x0b] = {.kind = DEC_RR, .reg1 = REG_BC},
	[0x0d] = {.kind = DEC_R, .reg1 = REG_C},
	[0x0e] = {.kind = LD_R_IMM8, .reg1 = REG_C},
	[0x0f] = {.kind = RRCA, .reg1 = REG_A},

	[0x11] = {.kind = LD_RR_IMM16, .reg1 = REG_DE},
	[0x12] = {.kind = LD_ARR_R, .reg1 = REG_DE, .reg2 = REG_A},
	[0x13] = {.kind = INC_RR, .reg1 = REG_DE},
	[0x15] = {.kind = DEC_R, .reg1 = REG_D},
	[0x16] = {.kind = LD_R_IMM8, .reg1 = REG_D},
	[0x17] = {.kind = RLA, .reg1 = REG_A},
	[0x18] = {.kind = JR_IMM8},
	[0x19] = {.kind = ADD_RR_RR, .reg1 = REG_HL, .reg2 = REG_DE},
	[0x1a] = {.kind = LD_R_ARR, .reg1 = REG_A, .reg2 = REG_DE},
	[0x1b] = {.kind = DEC_RR, .reg1 = REG_DE},
	[0x1d] = {.kind = DEC_R, .reg1 = REG_E},
	[0x1e] = {.kind = LD_R_IMM8, .reg1 = REG_E},
	[0x1f] = {.kind = RRA, .reg1 = REG_A},

	[0x20] = {.kind = JR_NZ_IMM8},
	[0x21] = {.kind = LD_RR_IMM16, .reg1 = REG_HL},
	[0x22] = {.kind = LD_ARRI_R, .reg1 = REG_HL, .reg2 = REG_A},
	[0x23] = {.kind = INC_RR, .reg1 = REG_HL},
	[0x25] = {.kind = DEC_R, .reg1 = REG_H},
	[0x26] = {.kind = LD_R_IMM8, .reg1 = REG_H},
	[0x28] = {.kind = JR_Z_IMM8},
	[0x29] = {.kind = ADD_RR_RR, .reg1 = REG_HL, .reg2 = REG_HL},
	[0x2a] = {.kind = LD_R_ARRI, .reg1 = REG_A, .reg2 = REG_HL},
	[0x2b] = {.kind = DEC_RR, .reg1 = REG_HL},
	[0x2d] = {.kind = DEC_R, .reg1 = REG_L},
	[0x2e] = {.kind = LD_R_IMM8, .reg1 = REG_L},

	[0x30] = {.kind = JR_NC_IMM8},
	[0x31] = {.kind = LD_RR_IMM16, .reg1 = REG_SP},
	[0x32] = {.kind = LD_ARRD_R, .reg1 = REG_HL, .reg2 = REG_A},
	[0x33] = {.kind = INC_RR, .reg1 = REG_SP},
	[0x35] = {.kind = DEC_ARR, .reg1 = REG_HL},
	[0x36] = {.kind = LD_ARR_IMM8, .reg1 = REG_HL},
	[0x38] = {.kind = JR_C_IMM8},
	[0x39] = {.kind = ADD_RR_RR, .reg1 = REG_HL, .reg2 = REG_SP},
	[0x3a] = {.kind = LD_R_ARRD, .reg1 = REG_A, .reg2 = REG_HL},
	[0x3b] = {.kind = DEC_RR, .reg1 = REG_SP},
	[0x3d] = {.kind = DEC_R, .reg1 = REG_A},
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

	[0x80] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_B},
	[0x81] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_C},
	[0x82] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_D},
	[0x83] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_E},
	[0x84] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_H},
	[0x85] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_L},
	[0x86] = {.kind = ADD_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0x87] = {.kind = ADD_R_R, .reg1 = REG_A, .reg2 = REG_A},
	[0x88] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_B},
	[0x89] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_C},
	[0x8a] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_D},
	[0x8b] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_E},
	[0x8c] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_H},
	[0x8d] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_L},
	[0x8e] = {.kind = ADC_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0x8f] = {.kind = ADC_R_R, .reg1 = REG_A, .reg2 = REG_A},

	[0x90] = {.kind = SUB_R_R, .reg1 = REG_A, .reg2 = REG_B},
	[0x91] = {.kind = SUB_R_R, .reg1 = REG_A, .reg2 = REG_C},
	[0x92] = {.kind = SUB_R_R, .reg1 = REG_A, .reg2 = REG_D},
	[0x93] = {.kind = SUB_R_R, .reg1 = REG_A, .reg2 = REG_E},
	[0x94] = {.kind = SUB_R_R, .reg1 = REG_A, .reg2 = REG_H},
	[0x95] = {.kind = SUB_R_R, .reg1 = REG_A, .reg2 = REG_L},
	[0x96] = {.kind = SUB_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0x97] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_A},
	[0x98] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_B},
	[0x99] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_C},
	[0x9a] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_D},
	[0x9b] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_E},
	[0x9c] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_H},
	[0x9d] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_L},
	[0x9e] = {.kind = SBC_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0x9f] = {.kind = SBC_R_R, .reg1 = REG_A, .reg2 = REG_A},

	[0xa0] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_B}, 
	[0xa1] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_C}, 
	[0xa2] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_D}, 
	[0xa3] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_E}, 
	[0xa4] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_H}, 
	[0xa5] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_L}, 
	[0xa6] = {.kind = AND_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0xa7] = {.kind = AND_R_R, .reg1 = REG_A, .reg2 = REG_A}, 
	[0xa8] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_B}, 
	[0xa9] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_C}, 
	[0xaa] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_D}, 
	[0xab] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_E}, 
	[0xac] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_H}, 
	[0xad] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_L}, 
	[0xae] = {.kind = XOR_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0xaf] = {.kind = XOR_R_R, .reg1 = REG_A, .reg2 = REG_A}, 

	[0xb0] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_B}, 
	[0xb1] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_C}, 
	[0xb2] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_D}, 
	[0xb3] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_E}, 
	[0xb4] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_H}, 
	[0xb5] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_L}, 
	[0xb6] = {.kind = OR_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0xb7] = {.kind = OR_R_R, .reg1 = REG_A, .reg2 = REG_A}, 
	[0xb8] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_B}, 
	[0xb9] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_C}, 
	[0xba] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_D}, 
	[0xbb] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_E}, 
	[0xbc] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_H}, 
	[0xbd] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_L}, 
	[0xbe] = {.kind = CP_R_ARR, .reg1 = REG_A, .reg2 = REG_HL},
	[0xbf] = {.kind = CP_R_R, .reg1 = REG_A, .reg2 = REG_A}, 

	[0xc0] = {.kind = RET_NZ},
	[0xc1] = {.kind = POP_RR, .reg1 = REG_BC},
	[0xc2] = {.kind = JP_NZ_IMM16},
	[0xc3] = {.kind = JP_IMM16},
	[0xc4] = {.kind = CALL_NZ_IMM16},
	[0xc5] = {.kind = PUSH_RR, .reg1 = REG_BC},
	[0xc6] = {.kind = ADD_R_IMM8, .reg1 = REG_A},
	[0xc8] = {.kind = RET_Z},
	[0xc9] = {.kind = RET},
	[0xca] = {.kind = JP_Z_IMM16},
	[0xcb] = {.kind = PRE_CB},
	[0xcc] = {.kind = CALL_Z_IMM16},
	[0xcd] = {.kind = CALL_IMM16},

	[0xd0] = {.kind = RET_NC},
	[0xd1] = {.kind = POP_RR, .reg1 = REG_DE},
	[0xd2] = {.kind = JP_NC_IMM16},
	[0xd4] = {.kind = CALL_NC_IMM16},
	[0xd5] = {.kind = PUSH_RR, .reg1 = REG_DE},
	[0xd6] = {.kind = SUB_R_IMM8, .reg1 = REG_A},
	[0xd8] = {.kind = RET_C},
	[0xd9] = {.kind = RETI},
	[0xda] = {.kind = JP_C_IMM16},
	[0xdc] = {.kind = CALL_C_IMM16},

	[0xe0] = {.kind = LDH_AIMM8_R, .reg2 = REG_A},
	[0xe1] = {.kind = POP_RR, .reg1 = REG_HL},
	[0xe2] = {.kind = LD_AR_R, .reg1 = REG_C, .reg2 = REG_A},
	[0xe5] = {.kind = PUSH_RR, .reg1 = REG_HL},
	[0xe8] = {.kind = ADD_RR_IMM8, .reg1 = REG_SP},
	[0xe9] = {.kind = JP_ARR},
	[0xea] = {.kind = LD_AIMM16_R, .reg2 = REG_A},
	[0xee] = {.kind = XOR_R_IMM8, .reg2 = REG_A},

	[0xf0] = {.kind = LDH_R_AIMM8, .reg1 = REG_A},
	[0xf1] = {.kind = POP_RR, .reg1 = REG_AF},
	[0xf2] = {.kind = LD_R_AR, .reg1 = REG_A, .reg2 = REG_C},
	[0xf3] = {.kind = DI},
	[0xf5] = {.kind = PUSH_RR, .reg1 = REG_AF},
	[0xf8] = {.kind = LD_RR_RR_IMM8, .reg1 = REG_SP, .reg2 = REG_HL},
	[0xf9] = {.kind = LD_RR_RR, .reg1 = REG_SP, .reg2 = REG_HL},
	[0xfa] = {.kind = LD_R_AIMM16, .reg1 = REG_A},
	[0xfe] = {.kind = CP_R_IMM8, .reg1 = REG_A},
};

static const char *const op_names[0x100] = {
	[0x00] = "NOP",
	[0x01] = "LD_RR_IMM16",
	[0x02] = "LD_ARR_R",
	[0x03] = "INC_RR",
	[0x05] = "DEC_R",
	[0x06] = "LD_R_IMM8",
	[0x09] = "ADD_RR_RR",
	[0x0a] = "LD_R_ARR",
	[0x0b] = "DEC_RR",
	[0x0d] = "DEC_R",
	[0x0e] = "LD_R_IMM8",

	[0x11] = "LD_RR_IMM16",
	[0x12] = "LD_ARR_R",
	[0x13] = "INC_RR",
	[0x15] = "DEC_R",
	[0x16] = "LD_R_IMM8",
	[0x18] = "JR_IMM8",
	[0x19] = "ADD_RR_RR",
	[0x1a] = "LD_R_ARR",
	[0x1b] = "DEC_RR",
	[0x1d] = "DEC_R",
	[0x1e] = "LD_R_IMM8",

	[0x20] = "JR_NZ_IMM8",
	[0x21] = "LD_RR_IMM16",
	[0x22] = "LD_ARRI_R",
	[0x23] = "INC_RR",
	[0x25] = "DEC_R",
	[0x26] = "LD_R_IMM8",
	[0x28] = "JR_Z_IMM8",
	[0x29] = "ADD_RR_RR",
	[0x2a] = "LD_R_ARRI",
	[0x2b] = "DEC_RR",
	[0x2d] = "DEC_R",
	[0x2e] = "LD_R_IMM8",

	[0x30] = "JR_NC_IMM8",
	[0x31] = "LD_RR_IMM16",
	[0x32] = "LD_ARRD_R",
	[0x33] = "INC_RR",
	[0x35] = "DEC_ARR",
	[0x36] = "LD_ARR_IMM8",
	[0x38] = "JR_C_IMM8",
	[0x39] = "ADD_RR_RR",
	[0x3a] = "LD_R_ARRD",
	[0x3b] = "DEC_RR",
	[0x3d] = "DEC_R",
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

	[0x80] = "ADD_R_R",
	[0x81] = "ADD_R_R",
	[0x82] = "ADD_R_R",
	[0x83] = "ADD_R_R",
	[0x84] = "ADD_R_R",
	[0x85] = "ADD_R_R",
	[0x86] = "ADD_R_ARR",
	[0x87] = "ADD_R_R",
	[0x88] = "ADC_R_R",
	[0x89] = "ADC_R_R",
	[0x8a] = "ADC_R_R",
	[0x8b] = "ADC_R_R",
	[0x8c] = "ADC_R_R",
	[0x8d] = "ADC_R_R",
	[0x8e] = "ADC_R_ARR",
	[0x8f] = "ADC_R_R",

	[0x90] = "SUB_R_R",
	[0x91] = "SUB_R_R",
	[0x92] = "SUB_R_R",
	[0x93] = "SUB_R_R",
	[0x94] = "SUB_R_R",
	[0x95] = "SUB_R_R",
	[0x96] = "SUB_R_ARR",
	[0x97] = "SBC_R_R",
	[0x98] = "SBC_R_R",
	[0x99] = "SBC_R_R",
	[0x9a] = "SBC_R_R",
	[0x9b] = "SBC_R_R",
	[0x9c] = "SBC_R_R",
	[0x9d] = "SBC_R_R",
	[0x9e] = "SBC_R_ARR",
	[0x9f] = "SBC_R_R",

	[0xa0] = "AND_R_R",
	[0xa1] = "AND_R_R",
	[0xa2] = "AND_R_R",
	[0xa3] = "AND_R_R",
	[0xa4] = "AND_R_R",
	[0xa5] = "AND_R_R",
	[0xa6] = "AND_R_ARR",
	[0xa7] = "AND_R_R",
	[0xa8] = "XOR_R_R",
	[0xa9] = "XOR_R_R",
	[0xaa] = "XOR_R_R",
	[0xab] = "XOR_R_R",
	[0xac] = "XOR_R_R",
	[0xad] = "XOR_R_R",
	[0xae] = "XOR_R_ARR",
	[0xaf] = "XOR_R_R",

	[0xb0] = "OR_R_R",
	[0xb1] = "OR_R_R",
	[0xb2] = "OR_R_R",
	[0xb3] = "OR_R_R",
	[0xb4] = "OR_R_R",
	[0xb5] = "OR_R_R",
	[0xb6] = "OR_R_ARR",
	[0xb7] = "OR_R_R",
	[0xb8] = "CP_R_R",
	[0xb9] = "CP_R_R",
	[0xba] = "CP_R_R",
	[0xbb] = "CP_R_R",
	[0xbc] = "CP_R_R",
	[0xbd] = "CP_R_R",
	[0xbe] = "CP_R_ARR",
	[0xbf] = "CP_R_R",

	[0xc0] = "RET_NZ",
	[0xc1] = "POP_RR",
	[0xc2] = "JP_NZ_IMM16",
	[0xc3] = "JP_IMM16",
	[0xc4] = "CALL_NZ_IMM16",
	[0xc5] = "PUSH_RR",
	[0xc6] = "ADD_R_IMM8",
	[0xc8] = "RET_Z",
	[0xc9] = "RET",
	[0xca] = "JP_Z_IMM16",
	[0xcb] = "PRE_CB",
	[0xcc] = "CALL_Z_IMM16",
	[0xcd] = "CALL_IMM16",

	[0xd0] = "RET_NC",
	[0xd1] = "POP_RR",
	[0xd2] = "JP_NC_IMM16",
	[0xd4] = "CALL_NC_IMM16",
	[0xd5] = "PUSH_RR",
	[0xd6] = "SUB_R_IMM8",
	[0xd8] = "RET_C",
	[0xd9] = "RETI",
	[0xda] = "JP_C_IMM16",
	[0xdc] = "CALL_C_IMM16",

	[0xe0] = "LDH_AIMM8_R",
	[0xe1] = "POP_RR",
	[0xe2] = "LD_AR_R",
	[0xe5] = "PUSH_RR",
	[0xe8] = "ADD_RR_IMM8",
	[0xe9] = "JP_ARR",
	[0xea] = "LD_AIMM16_R",
	[0xee] = "XOR_R_IMM8",

	[0xf0] = "LDH_R_AIMM8",
	[0xf1] = "POP_RR",
	[0xf2] = "LD_R_AR",
	[0xf3] = "DI",
	[0xf5] = "PUSH_RR",
	[0xf8] = "LD_RR_RR_IMM8",
	[0xf9] = "LD_RR_RR",
	[0xfa] = "LD_R_AIMM16",
	[0xfe] = "CP_R_IMM8",
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
