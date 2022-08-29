#include <stdio.h>
#include <assert.h>
#include "cpu.h"
#include "bus.h"

// https://gbdev.io/pandocs/CPU_Registers_and_Flags.html

#define BIT(a, n) (((a) & (1 << (n))) ? 1 : 0)
#define SET_BIT(a, n, on) a = (on ? ((a) | (1 << (n))) : ((a) & ~(1 << (n))))
#define FLAG_Z_BIT 7
#define FLAG_N_BIT 6
#define FLAG_H_BIT 5
#define FLAG_C_BIT 4
#define LO(a) a
#define HI(a) a << 8

#if 0
static uint8_t next_imm8(Cpu *cpu, const Cartridge *cart)
{
	uint8_t imm = bus_read(cart, cpu->regs.pc++);
	return imm;
}
#endif

static uint16_t next_imm16(Cpu *cpu, const Cartridge *cart)
{
	uint16_t lo = bus_read(cart, cpu->regs.pc++);
	uint16_t hi = bus_read(cart, cpu->regs.pc++);
	return LO(lo) | HI(hi);
}

static uint16_t read_reg16(const Cpu *cpu, Reg reg)
{
	switch (reg) {
		case REG_NONE:
			assert(0);
			break;
		case REG_A:
			return HI((uint16_t)cpu->regs.a);
		case REG_F:
			assert(0);
			break;
		case REG_B:
			return HI((uint16_t)cpu->regs.b);
		case REG_C:
			return LO((uint16_t)cpu->regs.c);
		case REG_D:
			return HI((uint16_t)cpu->regs.d);
		case REG_E:
			return LO((uint16_t)cpu->regs.e);
		case REG_H:
			return HI((uint16_t)cpu->regs.h);
		case REG_L:
			return LO((uint16_t)cpu->regs.l);
		case REG_AF:
			return HI((uint16_t)cpu->regs.a)
				& LO((uint16_t)cpu->regs.f);
		case REG_BC:
			return HI((uint16_t)cpu->regs.b)
				& LO((uint16_t)cpu->regs.c);
		case REG_DE:
			return HI((uint16_t)cpu->regs.d)
				& LO((uint16_t)cpu->regs.e);
		case REG_HL:
			return HI((uint16_t)cpu->regs.h)
				& LO((uint16_t)cpu->regs.l);
		case REG_SP:
			return cpu->regs.sp;
		case REG_PC:
			return cpu->regs.pc;
		default:
			assert(0);
			break;
	}
}

static uint8_t read_reg8(const Cpu *cpu, Reg reg)
{
	switch (reg) {
		case REG_NONE:
			assert(0);
			break;
		case REG_A:
		case REG_B:
		case REG_D:
		case REG_H:
			return read_reg16(cpu, reg) >> 8;
		case REG_F:
		case REG_C:
		case REG_E:
		case REG_L:
			return read_reg16(cpu, reg);
		case REG_AF:
		case REG_BC:
		case REG_DE:
		case REG_HL:
		case REG_SP:
		case REG_PC:
			assert(0);
			break;
		default:
			assert(0);
			break;
	}
}

static void write_reg8(Cpu *cpu, Reg reg, uint8_t byte)
{
	switch (reg) {
		case REG_NONE:
			assert(0);
			break;
		case REG_A:
			cpu->regs.a = byte;
			break;
		case REG_B:
			cpu->regs.b = byte;
			break;
		case REG_D:
			cpu->regs.d = byte;
			break;
		case REG_H:
			cpu->regs.h = byte;
			break;
		case REG_F:
			cpu->regs.f = byte;
			break;
		case REG_C:
			cpu->regs.c = byte;
			break;
		case REG_E:
			cpu->regs.e = byte;
			break;
		case REG_L:
			cpu->regs.l = byte;
			break;
		case REG_AF:
		case REG_BC:
		case REG_DE:
		case REG_HL:
		case REG_SP:
		case REG_PC:
			printf("Expected 8 bit register, got 16\n");
			exit(1);
			break;
		default:
			assert(0);
			break;
	}
}

static void write_reg16(Cpu *cpu, Reg reg, uint16_t word)
{
	switch (reg) {
		case REG_NONE:
			assert(0);
			break;
		case REG_A:
		case REG_B:
		case REG_D:
		case REG_H:
		case REG_F:
		case REG_C:
		case REG_E:
		case REG_L:
			printf("Expected 16 bit register, got 8\n");
			exit(1);
			break;
		case REG_AF:
			write_reg8(cpu, REG_A, word >> 8);
			write_reg8(cpu, REG_F, word);
			break;
		case REG_BC:
			write_reg8(cpu, REG_B, word >> 8);
			write_reg8(cpu, REG_C, word);
			break;
		case REG_DE:
			write_reg8(cpu, REG_D, word >> 8);
			write_reg8(cpu, REG_E, word);
			break;
		case REG_HL:
			write_reg8(cpu, REG_H, word >> 8);
			write_reg8(cpu, REG_L, word);
			break;
		case REG_SP:
			cpu->regs.sp = word;
			break;
		case REG_PC:
			cpu->regs.pc = word;
			break;
		default:
			assert(0);
			break;
	}
}

/*
 * 0: set to 0
 * 1: set to 1
 * 2: do nothing
 */
static void set_flags(Cpu *cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c)
{
	if (2 != z) {
		SET_BIT(cpu->regs.f, FLAG_Z_BIT, z);	
	}
	if (2 != n) {
		SET_BIT(cpu->regs.f, FLAG_N_BIT, n);	
	}
	if (2 != h) {
		SET_BIT(cpu->regs.f, FLAG_H_BIT, h);	
	}
	if (2 != c) {
		SET_BIT(cpu->regs.f, FLAG_C_BIT, c);	
	}
}

static void op_jmp(Cpu *cpu, const Cartridge *cart)
{
	uint16_t pc = cpu->regs.pc;
	switch (cpu->op.kind) {
		case JP_IMM16:
			pc = next_imm16(cpu, cart);
			break;
		case JP_AIMM16:
			assert(0);
			break;
		case JP_Z_IMM16:
			if (BIT(cpu->regs.f, FLAG_Z_BIT)) {
				pc = next_imm16(cpu, cart);
			}
			break;
		case JP_C_IMM16:
			if (BIT(cpu->regs.f, FLAG_C_BIT)) {
				pc = next_imm16(cpu, cart);
			}
			break;
		case JP_NZ_IMM16:
			if (!BIT(cpu->regs.f, FLAG_Z_BIT)) {
				pc = next_imm16(cpu, cart);
			}
			break;
		case JP_NC_IMM16:
			if (!BIT(cpu->regs.f, FLAG_C_BIT)) {
				pc = next_imm16(cpu, cart);
			}
			break;
		case JR_Z_IMM8:
		case JR_C_IMM8:
		case JR_NZ_IMM8:
		case JR_NC_IMM8:
			assert(0);
			break;
		default:
			assert(0);
			break;
	}
	cpu->regs.pc = pc;
}

static void op_xor_reg(Cpu *cpu)
{
	assert(XOR_R == cpu->op.kind);
	uint16_t tmp = 0;
	switch (cpu->op.reg1) {
		case REG_NONE:
			assert(0);
			break;
		case REG_A:
		case REG_F:
		case REG_B:
		case REG_C:
		case REG_D:
		case REG_E:
		case REG_H:
		case REG_L:
			tmp = read_reg8(cpu, cpu->op.reg1);
			write_reg8(cpu, cpu->op.reg1, tmp ^ tmp);
			break;
		case REG_AF:
		case REG_BC:
		case REG_DE:
		case REG_HL:
		case REG_SP:
		case REG_PC:
			tmp = read_reg16(cpu, cpu->op.reg1);
			write_reg16(cpu, cpu->op.reg1, tmp ^ tmp);
			break;
	}
}

static void op_xor(Cpu *cpu)
{
	switch (cpu->op.kind) {
		case XOR_R:
			op_xor_reg(cpu);
			break;
		case XOR_IMM8:
		case XOR_ARR:
			assert(0);
			break;
		default:
			assert(0);
			break;
	}
	set_flags(cpu, 1, 0, 0, 0);
}

#if 0
static void op_ld_reg_reg(Cpu *cpu)
{

}

static void op_ld(Cpu *cpu, const Cartridge *cart)
{
	switch (cpu->op.kind) {
		case LD_R_R:
			op_ld_reg_reg(cpu);
			break;
		case LD_R_IMM8:
		case LD_R_AIMM16:
		case LD_AR_R:
		case LD_RR_IMM16:
		case LD_R_AR:
		case LD_ARR_IMM8:
		case LD_ARR_R:
		case LD_ARRI_R:
		case LD_ARRD_R:
		case LD_AIMM16_R:
		case LD_AIMM16_RR:
		case LD_R_ARR:
		case LD_R_ARRI:
		case LD_R_ARRD:
		case LD_RR_RR_IMM8:
		case LDH_AIMM8_R:
		case LDH_R_AIMM8:
			assert(0);
			break;
		default:
			assert(0);
			break;
	}
}
#endif

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
			/* TODO: supposed to disable something, but I don't
			   know what exactly */
			break;
		case EI:
			assert(0);
			break;
		case LD_R_R:
		case LD_R_IMM8:
		case LD_R_AIMM16:
		case LD_AR_R:
		case LD_RR_IMM16:
		case LD_R_AR:
		case LD_ARR_IMM8:
		case LD_ARR_R:
		case LD_ARRI_R:
		case LD_ARRD_R:
		case LD_AIMM16_R:
		case LD_AIMM16_RR:
		case LD_R_ARR:
		case LD_R_ARRI:
		case LD_R_ARRD:
		case LD_RR_RR_IMM8:
		case LDH_AIMM8_R:
		case LDH_R_AIMM8:
			assert(0);
			break;
		case ADD_R_R:
		case ADD_R_IMM8:
		case ADD_RR_IMM8:
		case ADD_RR_RR:
		case ADD_R_ARR:
			assert(0);
			break;
		case SUB_R:
		case SUB_IMM8:
		case SUB_ARR:
			assert(0);
			break;
		case ADC_R_R:
		case ADC_R_IMM8:
		case ADC_R_ARR:
			assert(0);
			break;
		case SBC_R_R:
		case SBC_R_IMM8:
		case SBC_R_ARR:
			assert(0);
			break;
		case INC_R:
		case INC_RR:
		case INC_ARR:
			assert(0);
			break;
		case DEC_R:
		case DEC_RR:
		case DEC_ARR:
			assert(0);
			break;
		case AND_R:
		case AND_IMM8:
		case AND_ARR:
			assert(0);
			break;
		case XOR_R:
		case XOR_IMM8:
		case XOR_ARR:
			op_xor(cpu);
			break;
		case OR_R:
		case OR_IMM8:
		case OR_ARR:
			assert(0);
			break;
		case CP_R:
		case CP_IMM8:
		case CP_ARR:
			assert(0);
			break;
		case RET:
		case RETI:
		case RET_Z:
		case RET_C:
		case RET_NZ:
		case RET_NC:
			assert(0);
			break;
		case POP_RR:
			assert(0);
			break;
		case RST_00:
		case RST_10:
		case RST_20:
		case RST_30:
		case RST_08:
		case RST_18:
		case RST_28:
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
		case JP_AIMM16:
		case JP_Z_IMM16:
		case JP_C_IMM16:
		case JP_NZ_IMM16:
		case JP_NC_IMM16:
		case JR_Z_IMM8:
		case JR_C_IMM8:
		case JR_NZ_IMM8:
		case JR_NC_IMM8:
			op_jmp(cpu, cart);
			break;
		case CALL_IMM16:
		case CALL_Z_IMM16:
		case CALL_C_IMM16:
		case CALL_NZ_IMM16:
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
	printf("sp: 0x%04x ", regs.sp);
	printf("pc: 0x%04x\n", regs.pc);
}

void cpu_print(const Cpu *cpu, const Cartridge *cart)
{
	print_regs(cpu->regs);
	printf("%s (%02x %02x %02x)\n", op_name(bus_read(cart, cpu->regs.pc)),
			bus_read(cart, cpu->regs.pc),
			bus_read(cart, cpu->regs.pc + 1),
			bus_read(cart, cpu->regs.pc + 2));
}
