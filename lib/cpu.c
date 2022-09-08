#include <stdio.h>
#include <assert.h>
#include "cpu.h"
#include "bus.h"
#include "stack.h"
#include "common.h"

// https://gbdev.io/pandocs/CPU_Registers_and_Flags.html

static unsigned is_8bit_reg(Reg reg)
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
			return 1;
		case REG_AF:
		case REG_BC:
		case REG_DE:
		case REG_HL:
		case REG_SP:
		case REG_PC:
			return 0;
		default:
			assert(0);
			return 0;
	}
}

static uint8_t next_imm8(Cpu *cpu, const Cartridge *cart)
{
	uint8_t imm = bus_read(cart, cpu->regs.pc++);
	return imm;
}

static uint16_t next_imm16(Cpu *cpu, const Cartridge *cart)
{
	uint16_t lo = bus_read(cart, cpu->regs.pc++);
	uint16_t hi = bus_read(cart, cpu->regs.pc++);
	return lo | HI_SHIFT(hi);
}

static uint16_t read_reg16(const Cpu *cpu, Reg reg)
{
	switch (reg) {
		case REG_NONE:
			assert(0);
			break;
		case REG_A:
			return HI_SHIFT(cpu->regs.a);
		case REG_F:
			assert(0);
			break;
		case REG_B:
			return HI_SHIFT(cpu->regs.b);
		case REG_C:
			return cpu->regs.c;
		case REG_D:
			return HI_SHIFT(cpu->regs.d);
		case REG_E:
			return cpu->regs.e;
		case REG_H:
			return HI_SHIFT(cpu->regs.h);
		case REG_L:
			return cpu->regs.l;
		case REG_AF:
			return HI_SHIFT(cpu->regs.a) | cpu->regs.f;
		case REG_BC:
			return HI_SHIFT(cpu->regs.b) | cpu->regs.c;
		case REG_DE:
			return HI_SHIFT(cpu->regs.d) | cpu->regs.e;
		case REG_HL:
			return HI_SHIFT(cpu->regs.h) | cpu->regs.l;
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
			return LO_SHIFT(read_reg16(cpu, reg));
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

static uint16_t read_reg(const Cpu *cpu, Reg reg)
{
	if (is_8bit_reg(reg)) {
		return read_reg8(cpu, reg);
	} else {
		return read_reg16(cpu, reg);
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
			write_reg8(cpu, REG_A, LO_SHIFT(word));
			write_reg8(cpu, REG_F, word);
			break;
		case REG_BC:
			write_reg8(cpu, REG_B, LO_SHIFT(word));
			write_reg8(cpu, REG_C, word);
			break;
		case REG_DE:
			write_reg8(cpu, REG_D, LO_SHIFT(word));
			write_reg8(cpu, REG_E, word);
			break;
		case REG_HL:
			write_reg8(cpu, REG_H, LO_SHIFT(word));
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

static void write_reg(Cpu *cpu, Reg reg, uint16_t value)
{
	if (is_8bit_reg(reg)) {
		write_reg8(cpu, reg, value);
	} else {
		write_reg16(cpu, reg, value);
	}
}

static void write_areg_reg(Cpu *cpu, Cartridge *cart)
{
	uint16_t reg1 = read_reg(cpu, cpu->op.reg1);
	uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
	assert(reg2 < 0x100);
	bus_write8(cart, reg1, reg2);
}

static void write_reg_areg(Cpu *cpu, Cartridge *cart)
{
	uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
	write_reg(cpu, cpu->op.reg1, bus_read(cart, reg2));
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

static unsigned flag_cond_met(const Cpu *cpu)
{
	switch (cpu->op.kind) {
		case JP_IMM16:
		case CALL_IMM16:
		case RET:
		case RETI:
			return 1;
		case JP_Z_IMM16:
		case CALL_Z_IMM16:
		case JR_Z_IMM8:
		case RET_Z:
			if (FLAG_Z) {
				return 1;
			}
			break;
		case JP_C_IMM16:
		case CALL_C_IMM16:
		case JR_C_IMM8:
		case RET_C:
			if (FLAG_C) {
				return 1;
			}
			break;
		case JP_NZ_IMM16:
		case CALL_NZ_IMM16:
		case JR_NZ_IMM8:
		case RET_NZ:
			if (!FLAG_Z) {
				return 1;
			}
			break;
		case JP_NC_IMM16:
		case CALL_NC_IMM16:
		case JR_NC_IMM8:
		case RET_NC:
			if (!FLAG_C) {
				return 1;
			}
			break;
		default:
			assert(0);
			return 0;
	}
	return 0;
}

static uint16_t get_rst_addr(OpKind kind)
{
	switch (kind) {
		case RST_00:
			return 0x00;
		case RST_10:
			return 0x10;
		case RST_20:
			return 0x20;
		case RST_30:
			return 0x30;
		case RST_08:
			return 0x08;
		case RST_18:
			return 0x18;
		case RST_28:
			return 0x28;
		case RST_38:
			return 0x38;
		default:
			assert(0);
			return 0;
	}
}

static void op_jmp(Cpu *cpu, const Cartridge *cart)
{
	uint16_t imm = 0;
	switch (cpu->op.kind) {
		case JP_IMM16:
		case CALL_IMM16:
			imm = next_imm16(cpu, cart);
			cpu->regs.pc = imm;
			break;
		case JP_ARR:
			assert(REG_HL == cpu->op.reg1);
			cpu->regs.pc = read_reg(cpu, cpu->op.reg1);
			break;
		case JP_Z_IMM16:
		case CALL_Z_IMM16:
		case JP_C_IMM16:
		case CALL_C_IMM16:
		case JP_NZ_IMM16:
		case CALL_NZ_IMM16:
		case JP_NC_IMM16:
		case CALL_NC_IMM16:
			imm = next_imm16(cpu, cart);
			if (flag_cond_met(cpu)) {
				cpu->regs.pc = imm;
			}
			break;
		case JR_IMM8:
			imm = next_imm8(cpu, cart);
			cpu->regs.pc += (int8_t)imm;
			break;
		case JR_Z_IMM8:
		case JR_C_IMM8:
		case JR_NZ_IMM8:
		case JR_NC_IMM8:
			imm = next_imm8(cpu, cart);
			if (flag_cond_met(cpu)) {
				cpu->regs.pc += (int8_t)imm;
			}
			break;
		case RETI:
			cpu->ime_flag = 1;
			imm = stack_pop(cpu, cart);
			cpu->regs.pc = imm;
			break;
		case RET:
			imm = stack_pop(cpu, cart);
			cpu->regs.pc = imm;
			break;
		case RET_Z:
		case RET_C:
		case RET_NZ:
		case RET_NC:
			if (flag_cond_met(cpu)) {
				imm = stack_pop(cpu, cart);
				cpu->regs.pc = imm;
			}
			break;
		case RST_00:
		case RST_10:
		case RST_20:
		case RST_30:
		case RST_08:
		case RST_18:
		case RST_28:
		case RST_38:
			cpu->regs.pc = get_rst_addr(cpu->op.kind);
			break;
		default:
			assert(0);
			break;
	}
}

/*
 * 3 means set it based on the expression
 */
#define SET_FLAG(op, f, F, cond) \
	uint8_t f = (3 == cond ? F##_FLAG(reg, val, op) : cond);

#define OP(op, reg_kind, write, z_val, n_val, h_val, c_val) \
	uint16_t reg = read_reg(cpu, reg_kind); \
	SET_FLAG(op, z, Z, z_val); \
	SET_FLAG(op, n, N, n_val); \
	SET_FLAG(op, h, H, h_val); \
	SET_FLAG(op, c, C, c_val); \
	if (write) { \
		write_reg(cpu, reg_kind, reg op val); \
	} \
	set_flags(cpu, z, n, h, c);

static void op_and(Cpu *cpu, uint16_t val)
{
	assert(REG_A == cpu->op.reg1);
	OP(&, cpu->op.reg1, 1, 3, 0, 1, 0);
}

static void op_or(Cpu *cpu, uint16_t val)
{
	assert(REG_A == cpu->op.reg1);
	OP(|, cpu->op.reg1, 1, 3, 0, 0, 0);
}

static void op_xor(Cpu *cpu, uint16_t val)
{
	assert(REG_A == cpu->op.reg1);
	OP(^, cpu->op.reg1, 1, 3, 0, 0, 0);
}

static void op_add(Cpu *cpu, Reg reg_kind, uint16_t val)
{
	switch (reg_kind) {
		case REG_A:
			{
				OP(+, reg_kind, 1, 3, 0, 3, 3);
				break;
			}
		case REG_HL:
			{
				OP(+, reg_kind, 1, 2, 0, 3, 3);
				break;
			}
		case REG_SP:
			{
				OP(+, reg_kind, 1, 0, 0, 3, 3);
				break;
			}
		default:
			assert(0);
			break;
	}
}

static void op_sub(Cpu *cpu, Reg reg_kind, uint16_t val)
{
	OP(-, reg_kind, 1, 3, 1, 3, 3);
}

static void op_cp(Cpu *cpu, uint16_t val)
{
	assert(REG_A == cpu->op.reg1);
	OP(-, cpu->op.reg1, 0, 3, 1, 3, 3);
}

static void add_to_addr(Cartridge *cart, uint16_t addr, uint8_t val)
{
	uint8_t new_val = bus_read(cart, addr);
	new_val += val;
	bus_write8(cart, addr, new_val);
}

static void op_inc(Cpu *cpu, Cartridge *cart)
{
	uint16_t val = 1;
	switch (cpu->op.kind) {
		case INC_R:
			{
				OP(+, cpu->op.reg1, 1, 3, 0, 3, 2);
				break;
			}
		case INC_RR:
		case LD_ARRI_R:
			{
				OP(+, cpu->op.reg1, 1, 2, 2, 2, 2);
				break;
			}
		case LD_R_ARRI:
			{
				OP(+, cpu->op.reg2, 1, 2, 2, 2, 2);
				break;
			}
		case INC_ARR:
			{
				uint16_t reg = read_reg(cpu, cpu->op.reg1);
				add_to_addr(cart, reg, 1);
				set_flags(cpu, Z_FLAG(reg, 1, +), 0,
						H_FLAG(reg, 1, +), 2);
				break;
			}
		default:
			assert(0);
			break;
	}
}

static void op_dec(Cpu *cpu, Cartridge *cart)
{
	uint16_t val = 1;
	switch (cpu->op.kind) {
		case DEC_R:
			{
				OP(-, cpu->op.reg1, 1, 3, 1, 3, 2);
				break;
			}
		case DEC_RR:
		case LD_ARRD_R:
			{
				OP(-, cpu->op.reg1, 1, 2, 2, 2, 2);
				break;
			}
		case LD_R_ARRD:
			{
				OP(-, cpu->op.reg2, 1, 2, 2, 2, 2);
				break;
			}
		case DEC_ARR:
			{
				uint16_t reg = read_reg(cpu, cpu->op.reg1);
				add_to_addr(cart, reg, -1);
				set_flags(cpu, Z_FLAG(reg, 1, -), 1,
						H_FLAG(reg, 1, -), 2);
				break;
			}
		default:
			assert(0);
			break;
	}
}

#undef SET_FLAG
#undef OP

static void op_ld(Cpu *cpu, Cartridge *cart)
{
	uint16_t reg1 = 0;
	uint16_t reg2 = 0;
	uint16_t imm = 0;
	switch (cpu->op.kind) {
		case LD_R_R:
			reg2 = read_reg(cpu, cpu->op.reg2);
			write_reg(cpu, cpu->op.reg1, reg2);
			break;
		case LD_R_IMM8:
			imm = next_imm8(cpu, cart);
			write_reg(cpu, cpu->op.reg1, imm);
			break;
		case LD_R_AIMM16:
			imm = bus_read(cart, next_imm16(cpu, cart));
			write_reg(cpu, cpu->op.reg1, imm);
			break;
		case LD_AR_R:
			reg1 = read_reg(cpu, cpu->op.reg1);
			reg2 = read_reg(cpu, cpu->op.reg2);
			assert(reg2 < 0x100);
			bus_write8(cart, 0xff00 | reg1, reg2);
			break;
		case LD_RR_IMM16:
			imm = next_imm16(cpu, cart);
			write_reg(cpu, cpu->op.reg1, imm);
			break;
		case LD_R_AR:
			reg2 = 0xff00 | read_reg(cpu, cpu->op.reg2);
			write_reg(cpu, cpu->op.reg1, bus_read(cart, reg2));
			break;
		case LD_ARR_IMM8:
			imm = next_imm8(cpu, cart);
			reg1 = read_reg(cpu, cpu->op.reg1);
			bus_write8(cart, reg1, imm);
			break;
		case LD_ARR_R:
			write_areg_reg(cpu, cart);
			break;
		case LD_ARRI_R:
			write_areg_reg(cpu, cart);
			op_inc(cpu, cart);
			break;
		case LD_ARRD_R:
			write_areg_reg(cpu, cart);
			op_dec(cpu, cart);
			break;
		case LD_AIMM16_R:
			imm = next_imm16(cpu, cart);
			reg2 = read_reg(cpu, cpu->op.reg2);
			assert(reg2 < 0x100);
			bus_write8(cart, imm, reg2);
			break;
		case LD_AIMM16_RR:
			imm = next_imm16(cpu, cart);
			reg2 = read_reg(cpu, cpu->op.reg2);
			bus_write16(cart, imm, reg2);
			break;
		case LD_R_ARR:
			write_reg_areg(cpu, cart);
			break;
		case LD_R_ARRI:
			write_reg_areg(cpu, cart);
			op_inc(cpu, cart);
			break;
		case LD_R_ARRD:
			write_reg_areg(cpu, cart);
			op_dec(cpu, cart);
			break;
		case LD_RR_RR_IMM8:
			imm = next_imm8(cpu, cart);
			reg2 = read_reg(cpu, cpu->op.reg2);
			write_reg(cpu, cpu->op.reg1, reg2 + (int8_t)imm);
			set_flags(cpu, 0, 0, H_FLAG(reg2, imm, +),
					C_FLAG(reg2, imm, +));
			break;
		case LD_RR_RR:
			reg2 = read_reg(cpu, cpu->op.reg2);
			write_reg(cpu, cpu->op.reg1, reg2);
			break;
		case LDH_AIMM8_R:
			imm = next_imm8(cpu, cart);
			reg2 = read_reg(cpu, cpu->op.reg2);
			assert(reg2 < 0x100);
			bus_write8(cart, 0xff00 | imm, reg2);
			break;
		case LDH_R_AIMM8:
			imm = 0xff00 | next_imm8(cpu, cart);
			write_reg(cpu, cpu->op.reg1, bus_read(cart, imm));
			break;
		default:
			assert(0);
			break;
	}
}

static Reg regs[] = {
	REG_B,
	REG_C,
	REG_D,
	REG_E,
	REG_H,
	REG_L,
	REG_HL,
	REG_A,
};

/*
 * first three bits of CB opcode determine the register
 */
static Reg decode_op(uint8_t op)
{
	return regs[op & 0x7];
}

static void cb_write(Cpu *cpu, Cartridge *cart, Reg reg, uint16_t data)
{
	if (REG_HL == reg) {
		bus_write8(cart, read_reg(cpu, reg), data);
	} else {
		write_reg(cpu, reg, data);
	}
}

static void op_cb(Cpu *cpu, Cartridge *cart)
{
	uint8_t op = next_imm8(cpu, cart);
	Reg reg_kind = decode_op(op);
	uint16_t reg = read_reg(cpu, reg_kind);
	uint16_t result = 0;
	uint8_t c = 0;

	/*
	 * there's a pattern in the opcodes to determine which bit should be
	 * checked
	 */
	uint8_t bit = (op >> 3) & 0x07;

	// top two bits determine op type
	uint8_t op_type = (op >> 6) & 0x03;

	switch (op_type) {
		case 0x01: // BIT
			set_flags(cpu, !BIT(reg, bit), 0, 1, 2);
			return;
		case 0x02: // RES
			result = reg & ~(1 << bit);
			cb_write(cpu, cart, reg_kind, result);
			return;
		case 0x03: // SET
			result = reg | (1 << bit);
			cb_write(cpu, cart, reg_kind, result);
			return;
		default:
			// not one of these op types, go to next switch
			break;
	}

// https://www.geeksforgeeks.org/rotate-instructions-in-8085/
	switch (bit) {
		case 0x00: // RLC
			c = (BIT(reg, 7) ? 1 : 0);
			result = (reg << 1) | (c ? 1 : 0);
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, c);
			return;
		case 0x01: // RRC
			c = BIT(reg, 0);
			result = (reg >> 1) | (c ? 1 << 7 : 0);
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, c);
			return;
		case 0x02: // RL
			c = (BIT(reg, 7) ? 1 : 0);
			result = (reg << 1) | (FLAG_C ? 1 : 0);
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, c);
			return;
		case 0x03: // RR
			c = BIT(reg, 0);
			result = (reg >> 1) | (FLAG_C ? 1 << 7 : 0);
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, c);
			return;
		case 0x04: // SLA
			c = (BIT(reg, 7) ? 1 : 0);
			result = reg << 1;
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, c);
			return;
		case 0x05: // SRA shift right arithmetic (b7=b7)
			result = (int8_t)reg >> 1;
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, 0);
			return;
		case 0x06: // SWAP
			result = ((reg & 0xf0) >> 4) | ((reg & 0xf) << 4);
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, 0);
			return;
		case 0x07: // SRL
			c = BIT(reg, 0);
			result = reg >> 1;
			cb_write(cpu, cart, reg_kind, result);
			set_flags(cpu, 0 == result, 0, 0, c);
			return;
	}
	assert(0);
}

static void op_push(Cpu *cpu, Cartridge *cart)
{
	Reg reg = cpu->op.reg1;
	assert(!is_8bit_reg(reg));
	uint16_t data = read_reg(cpu, cpu->op.reg1);
	stack_push(cpu, cart, data);
}

/*
 * from complete technical reference:
 POP rr
 Pops to the 16-bit register rr, data from the stack memory.
 This instruction does not do calculations that affect flags,
 but POP AF completely replaces the F register
 value, so all flags are changed based on the 8-bit data that is read
 from memory.
 */
static void op_pop(Cpu *cpu, Cartridge *cart)
{
	Reg reg = cpu->op.reg1;
	assert(!is_8bit_reg(reg));
	uint16_t data = stack_pop(cpu, cart);
	write_reg(cpu, reg, data);
	// TODO: still to check whether F flags need to be adapted here
}

static void op_rot(Cpu *cpu)
{
	assert(REG_A == cpu->op.reg1);
	uint16_t reg = read_reg(cpu, cpu->op.reg1);
	uint16_t result = 0;
	uint8_t c = 0;
	switch (cpu->op.kind) {
		case RLCA_R:
			c = (BIT(reg, 7) ? 1 : 0);
			result = (reg << 1) | (BIT(reg, 7) ? 1 : 0);
			break;
		case RRCA_R:
			c = BIT(reg, 0);
			result = (reg >> 1) | (BIT(reg, 0) ? 1 << 7 : 0);
			break;
		case RLA_R:
			c = (BIT(reg, 7) ? 1 : 0);
			result = (reg << 1) | FLAG_C;
			break;
		case RRA_R:
			c = BIT(reg, 0);
			result = (reg >> 1) | (FLAG_C << 7);
			break;
		default:
			assert(0);
			break;
	}
	write_reg(cpu, cpu->op.reg1, result);
	set_flags(cpu, 0, 0, 0, c);
}

static void op_daa(Cpu *cpu)
{
	assert(REG_A == cpu->op.reg1);
	uint16_t reg = read_reg(cpu, cpu->op.reg1);
	uint16_t result = 0;
	uint8_t c = 0;

	if (FLAG_H || (!FLAG_N && (reg & 0xf) > 9)) {
		result = 6;
	}

	if (FLAG_C || (!FLAG_N && reg > 0x99)) {
		result |= 0x60;
	}

	result = FLAG_N ? reg - result : reg + result;
	write_reg(cpu, cpu->op.reg1, result);

	c = (reg & 0xff << 8) != (result & 0xff << 8);
	set_flags(cpu, 0 == result, 2, 0, c);
}

static void op_cpl(Cpu *cpu)
{
	assert(REG_A == cpu->op.reg1);
	uint16_t reg = read_reg(cpu, cpu->op.reg1);
	uint16_t result = ~reg;
	write_reg(cpu, cpu->op.reg1, result);
	set_flags(cpu, 2, 1, 1, 2);
}
static void op_scf(Cpu *cpu)
{
	set_flags(cpu, 2, 0, 0, 1);
}

static void op_ccf(Cpu *cpu)
{
	set_flags(cpu, 2, 0, 0, FLAG_C ^ 1);
}

void next_op(Cpu *cpu, Cartridge *cart)
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
			cpu->halted = 1;
			break;
		case PRE_CB:
			op_cb(cpu, cart);
			break;
		case DI:
			cpu->ime_flag = 0;
			break;
		case EI:
			cpu->ime_flag = 1;
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
		case LD_RR_RR:
		case LDH_AIMM8_R:
		case LDH_R_AIMM8:
			op_ld(cpu, cart);
			break;
		case ADD_R_R:
		case ADD_RR_RR:
			op_add(cpu, cpu->op.reg1, read_reg(cpu, cpu->op.reg2));
			break;
		case ADD_R_IMM8:
		case ADD_RR_IMM8:
			op_add(cpu, cpu->op.reg1, next_imm8(cpu, cart));
			break;
		case ADD_R_ARR:
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_add(cpu, cpu->op.reg1, val);
				break;
			}
		case SUB_R_R:
			op_sub(cpu, cpu->op.reg1, read_reg(cpu, cpu->op.reg2));
			break;
		case SUB_R_IMM8:
			op_sub(cpu, cpu->op.reg1, next_imm8(cpu, cart));
			break;
		case SUB_R_ARR:
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_sub(cpu, cpu->op.reg1, val);
				break;
			}
			break;
		case ADC_R_R:
			assert(REG_A == cpu->op.reg1);
			op_add(cpu, cpu->op.reg1, read_reg(cpu, cpu->op.reg2)
					+ FLAG_C);
			break;
		case ADC_R_IMM8:
			assert(REG_A == cpu->op.reg1);
			op_add(cpu, cpu->op.reg1, next_imm8(cpu, cart)
					+ FLAG_C);
			break;
		case ADC_R_ARR:
			assert(REG_A == cpu->op.reg1);
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_add(cpu, cpu->op.reg1, val + FLAG_C);
			}
			break;
		case SBC_R_R:
			assert(REG_A == cpu->op.reg1);
			op_sub(cpu, cpu->op.reg1, read_reg(cpu, cpu->op.reg2)
					- FLAG_C);
			break;
		case SBC_R_IMM8:
			assert(REG_A == cpu->op.reg1);
			op_sub(cpu, cpu->op.reg1, next_imm8(cpu, cart)
					- FLAG_C);
			break;
		case SBC_R_ARR:
			assert(REG_A == cpu->op.reg1);
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_sub(cpu, cpu->op.reg1, val - FLAG_C);
			}
			break;
		case INC_R:
		case INC_RR:
		case INC_ARR:
			op_inc(cpu, cart);
			break;
		case DEC_R:
		case DEC_RR:
		case DEC_ARR:
			op_dec(cpu, cart);
			break;
		case AND_R_R:
			assert(REG_A == cpu->op.reg1);
			op_and(cpu, read_reg(cpu, cpu->op.reg2));
			break;
		case AND_R_IMM8:
			assert(REG_A == cpu->op.reg1);
			op_and(cpu, next_imm8(cpu, cart));
			break;
		case AND_R_ARR:
			assert(REG_A == cpu->op.reg1);
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_and(cpu, val);
			}
			break;
		case XOR_R_R:
			assert(REG_A == cpu->op.reg1);
			op_xor(cpu, read_reg(cpu, cpu->op.reg2));
			break;
		case XOR_R_IMM8:
			assert(REG_A == cpu->op.reg1);
			op_xor(cpu, next_imm8(cpu, cart));
			break;
		case XOR_R_ARR:
			assert(REG_A == cpu->op.reg1);
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_xor(cpu, val);
			}
			break;
		case OR_R_R:
			assert(REG_A == cpu->op.reg1);
			op_or(cpu, read_reg(cpu, cpu->op.reg2));
			break;
		case OR_R_IMM8:
			assert(REG_A == cpu->op.reg1);
			op_or(cpu, next_imm8(cpu, cart));
			break;
		case OR_R_ARR:
			assert(REG_A == cpu->op.reg1);
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_or(cpu, val);
			}
			break;
		case CP_R_R:
			assert(REG_A == cpu->op.reg1);
			op_cp(cpu, read_reg(cpu, cpu->op.reg2));
			break;
		case CP_R_IMM8:
			assert(REG_A == cpu->op.reg1);
			op_cp(cpu, next_imm8(cpu, cart));
			break;
		case CP_R_ARR:
			assert(REG_A == cpu->op.reg1);
			{
				uint16_t reg2 = read_reg(cpu, cpu->op.reg2);
				uint16_t val = bus_read(cart, reg2);
				op_cp(cpu, val);
			}
			break;
		case RETI:
		case RET:
		case RET_Z:
		case RET_C:
		case RET_NZ:
		case RET_NC:
			op_jmp(cpu, cart);
			break;
		case POP_RR:
			op_pop(cpu, cart);
			break;
		case RST_00:
		case RST_10:
		case RST_20:
		case RST_30:
		case RST_08:
		case RST_18:
		case RST_28:
		case RST_38:
			op_jmp(cpu, cart);
			break;
		case DAA_R:
			op_daa(cpu);
			break;
		case SCF:
			op_scf(cpu);
			break;
		case CPL_R:
			op_cpl(cpu);
			break;
		case CCF:
			op_ccf(cpu);
			break;
		case JP_IMM16:
		case JP_ARR:
		case JP_Z_IMM16:
		case JP_C_IMM16:
		case JP_NZ_IMM16:
		case JP_NC_IMM16:
		case JR_IMM8:
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
			if (flag_cond_met(cpu)) {
				stack_push(cpu, cart, cpu->regs.pc + 2);
			}
			op_jmp(cpu, cart);
			break;
		case PUSH_RR:
			op_push(cpu, cart);
			break;
		case RLCA_R:
		case RRCA_R:
		case RLA_R:
		case RRA_R:
			op_rot(cpu);
			break;
	}
}

static void print_flags(uint8_t f)
{
	printf("F: ");
	BIT(f, FLAG_Z_BIT) ? printf("Z") : printf("-");	
	BIT(f, FLAG_N_BIT) ? printf("N") : printf("-");	
	BIT(f, FLAG_H_BIT) ? printf("H") : printf("-");	
	BIT(f, FLAG_C_BIT) ? printf("C") : printf("-");	
	printf(" ");
}

static void print_regs(struct registers regs)
{
	printf("A: %02x ", regs.a);
	print_flags(regs.f);
	printf("BC: %02x%02x ", regs.b, regs.c);
	printf("DE %02x%02x ", regs.d, regs.e);
	printf("HL: %02x%02x ", regs.h, regs.l);
	printf("SP: %04x ", regs.sp);
	printf("PC: %04x\n", regs.pc);
}

void cpu_print(const Cpu *cpu, const Cartridge *cart)
{
	printf("%-13s (%02x %02x %02x) ", op_name(bus_read(cart, cpu->regs.pc)),
			bus_read(cart, cpu->regs.pc),
			bus_read(cart, cpu->regs.pc + 1),
			bus_read(cart, cpu->regs.pc + 2));
	print_regs(cpu->regs);
}

// https://gbdev.io/pandocs/Power_Up_Sequence.html?highlight=d8#cpu-registers

static struct registers init_regs(void)
{
	return (struct registers) {.a = 0x01,
		.f = 0xB0, // Z-HC
		.b = 0x00,
		.c = 0x13,
		.d = 0x00,
		.e = 0xd8,
		.h = 0x01,
		.l = 0x4d,
		.pc = 0x100,
		.sp = 0xfffe,
	};
}

Cpu *cpu_init(void)
{
	Cpu *cpu = malloc(sizeof(*cpu));
	*cpu = (Cpu){.regs = init_regs(), 
		.ime_flag = 0, .halted = 0, .ie_reg = 0, .if_reg = 0
	};
	return cpu;
}
