#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

typedef enum {
	NONE,
	NOP,
	STOP,
	HALT,
	PRE_CB,
	DI,
	EI,

	LD_R_R,
	LD_R_IMM8,
	LD_R_AIMM16,
	LD_AR_R,
	LD_RR_IMM16,
	LD_R_AR,
	LD_ARR_IMM8,
	LD_ARR_R,
	LD_ARRI_R,
	LD_ARRD_R,
	LD_AIMM16_R,
	LD_AIMM16_RR,
	LD_R_ARR,
	LD_R_ARRI,
	LD_R_ARRD,
	LD_RR_RR_IMM8,
	LD_RR_RR,
	LDH_AIMM8_R,
	LDH_R_AIMM8,

	ADD_R_R,
	ADD_R_IMM8,
	ADD_RR_IMM8,
	ADD_RR_RR,
	ADD_R_ARR,

	SUB_R,
	SUB_IMM8,
	SUB_ARR,

	ADC_R_R,
	ADC_R_IMM8,
	ADC_R_ARR,

	SBC_R_R,
	SBC_R_IMM8,
	SBC_R_ARR,

	INC_R,
	INC_RR,
	INC_ARR,

	DEC_R,
	DEC_RR,
	DEC_ARR,

	AND_R,
	AND_IMM8,
	AND_ARR,

	XOR_R,
	XOR_IMM8,
	XOR_ARR,

	OR_R,
	OR_IMM8,
	OR_ARR,

	CP_R,
	CP_IMM8,
	CP_ARR,

	RET,
	RETI,
	RET_Z,
	RET_C,
	RET_NZ,
	RET_NC,

	POP_RR,

	RST_00,
	RST_10,
	RST_20,
	RST_30,
	RST_08,
	RST_18,
	RST_28,
	RST_38,

	DAA,
	SCF,
	CPL,
	CPF,

	JP_IMM16,
	JP_AIMM16,
	JP_Z_IMM16,
	JP_C_IMM16,
	JP_NZ_IMM16,
	JP_NC_IMM16,

	JR_Z_IMM8,
	JR_C_IMM8,
	JR_NZ_IMM8,
	JR_NC_IMM8,

	CALL_IMM16,
	CALL_Z_IMM16,
	CALL_C_IMM16,
	CALL_NZ_IMM16,
	CALL_NC_IMM16,

	PUSH_RR,

	RLCA,
	RRCA,

	// TODO: CB
} OpKind;

typedef enum {
	REG_NONE,
	REG_A,
	REG_F,
	REG_B,
	REG_C,
	REG_D,
	REG_E,
	REG_H,
	REG_L,
	REG_AF,
	REG_BC,
	REG_DE,
	REG_HL,
	REG_SP,
	REG_PC,
} Reg;

typedef struct Op {
	OpKind kind;
	Reg reg1;
	Reg reg2;
} Op;

Op get_op_from_opcode(uint8_t opcode);
const char *op_name(uint8_t opcode);

#endif
