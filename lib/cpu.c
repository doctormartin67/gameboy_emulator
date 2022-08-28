#include <stdio.h>
#include <assert.h>
#include "cpu.h"
#include "bus.h"

void next_op(Cpu *cpu, const Cartridge *cart)
{
	cpu->opcode = bus_read(cart, cpu->regs.pc++);
	cpu->op = get_op_from_opcode(cpu->opcode);

	switch (cpu->op.kind) {
		case NONE:
			assert(0);
			break;
		case NOP:
			// do nothing
			break;
		case STOP:
			assert(0);
			break;
		case HALT:
			assert(0);
			break;
		case PRE_CB:
			assert(0);
			break;
		case DI:
			assert(0);
			break;
		case EI:
			assert(0);
			break;
		case LD_R_R:
			assert(0);
			break;
		case LD_R_IMM8:
			assert(0);
			break;
		case LD_R_AIMM16:
			assert(0);
			break;
		case LD_AR_R:
			assert(0);
			break;
		case LD_RR_IMM16:
			assert(0);
			break;
		case LD_R_AR:
			assert(0);
			break;
		case LD_ARR_IMM8:
			assert(0);
			break;
		case LD_ARR_R:
			assert(0);
			break;
		case LD_ARRI_R:
			assert(0);
			break;
		case LD_ARRD_R:
			assert(0);
			break;
		case LD_AIMM16_R:
			assert(0);
			break;
		case LD_AIMM16_RR:
			assert(0);
			break;
		case LD_R_ARR:
			assert(0);
			break;
		case LD_R_ARRI:
			assert(0);
			break;
		case LD_R_ARRD:
			assert(0);
			break;
		case LD_RR_RR_IMM8:
			assert(0);
			break;
		case LDH_AIMM8_R:
			assert(0);
			break;
		case LDH_R_AIMM8:
			assert(0);
			break;
		case ADD_R_R:
			assert(0);
			break;
		case ADD_R_IMM8:
			assert(0);
			break;
		case ADD_RR_IMM8:
			assert(0);
			break;
		case ADD_RR_RR:
			assert(0);
			break;
		case ADD_R_ARR:
			assert(0);
			break;
		case SUB_R:
			assert(0);
			break;
		case SUB_IMM8:
			assert(0);
			break;
		case SUB_ARR:
			assert(0);
			break;
		case ADC_R_R:
			assert(0);
			break;
		case ADC_R_IMM8:
			assert(0);
			break;
		case ADC_R_ARR:
			assert(0);
			break;
		case SBC_R_R:
			assert(0);
			break;
		case SBC_R_IMM8:
			assert(0);
			break;
		case SBC_R_ARR:
			assert(0);
			break;
		case INC_R:
			assert(0);
			break;
		case INC_RR:
			assert(0);
			break;
		case INC_ARR:
			assert(0);
			break;
		case DEC_R:
			assert(0);
			break;
		case DEC_RR:
			assert(0);
			break;
		case DEC_ARR:
			assert(0);
			break;
		case AND_R:
			assert(0);
			break;
		case AND_IMM8:
			assert(0);
			break;
		case AND_ARR:
			assert(0);
			break;
		case XOR_R:
			assert(0);
			break;
		case XOR_IMM8:
			assert(0);
			break;
		case XOR_ARR:
			assert(0);
			break;
		case OR_R:
			assert(0);
			break;
		case OR_IMM8:
			assert(0);
			break;
		case OR_ARR:
			assert(0);
			break;
		case CP_R:
			assert(0);
			break;
		case CP_IMM8:
			assert(0);
			break;
		case CP_ARR:
			assert(0);
			break;
		case RET:
			assert(0);
			break;
		case RETI:
			assert(0);
			break;
		case RET_Z:
			assert(0);
			break;
		case RET_C:
			assert(0);
			break;
		case RET_NZ:
			assert(0);
			break;
		case RET_NC:
			assert(0);
			break;
		case POP_RR:
			assert(0);
			break;
		case RST_00:
			assert(0);
			break;
		case RST_10:
			assert(0);
			break;
		case RST_20:
			assert(0);
			break;
		case RST_30:
			assert(0);
			break;
		case RST_08:
			assert(0);
			break;
		case RST_18:
			assert(0);
			break;
		case RST_28:
			assert(0);
			break;
		case RST_38:
			assert(0);
			break;
		case DAA:
			assert(0);
			break;
		case SCF:
			assert(0);
			break;
		case CPL:
			assert(0);
			break;
		case CPF:
			assert(0);
			break;
		case JP_IMM16:
			assert(0);
			break;
		case JP_AIMM16:
			assert(0);
			break;
		case JP_Z_IMM16:
			assert(0);
			break;
		case JP_C_IMM16:
			assert(0);
			break;
		case JP_NZ_IMM16:
			assert(0);
			break;
		case JP_NC_IMM16:
			assert(0);
			break;
		case JR_Z_IMM8:
			assert(0);
			break;
		case JR_C_IMM8:
			assert(0);
			break;
		case JR_NZ_IMM8:
			assert(0);
			break;
		case JR_NC_IMM8:
			assert(0);
			break;
		case CALL_IMM16:
			assert(0);
			break;
		case CALL_Z_IMM16:
			assert(0);
			break;
		case CALL_C_IMM16:
			assert(0);
			break;
		case CALL_NZ_IMM16:
			assert(0);
			break;
		case CALL_NC_IMM16:
			assert(0);
			break;
		case PUSH_RR:
			assert(0);
			break;
		case RLCA:
			assert(0);
			break;
		case RRCA:
			assert(0);
			break;
	}
}

static void print_regs(struct registers regs)
{
	printf("a: 0x%04x ", regs.a);
	printf("f: 0x%04x ", regs.f);
	printf("b: 0x%04x ", regs.b);
	printf("c: 0x%04x\n", regs.c);
	printf("d: 0x%04x ", regs.d);
	printf("e: 0x%04x ", regs.e);
	printf("h: 0x%04x ", regs.h);
	printf("l: 0x%04x\n", regs.l);
	printf("sp: 0x%04x\n", regs.sp);
	printf("pc: 0x%04x\n", regs.pc);
}

void cpu_print(const Cpu *cpu)
{
	print_regs(cpu->regs);
	printf("Opcode: 0x%04x\n", cpu->opcode);
}
