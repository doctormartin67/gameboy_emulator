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
		assert(value < 0x100);
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

static uint8_t z_flag(uint16_t word)
{
	return !(word);
}

static uint8_t n_flag(uint16_t word)
{
	return BIT(word, 15); // negative bit
}

/*
 * h flag set <=> high nibble is different than low nibble after operation
 */

static uint8_t h_flag(uint16_t old_word, uint16_t new_word)
{
	return (old_word & 0xf << 4) != (new_word & 0xf << 4);
}

/*
 * c flag set <=> high byte is different than low byte after operation
 */

static uint8_t c_flag(uint16_t old_word, uint16_t new_word)
{
	return (old_word & 0xff << 8) != (new_word & 0xff << 8);
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
		case JP_Z_IMM16:
		case CALL_Z_IMM16:
		case JR_Z_IMM8:
		case RET_Z:
			if (BIT(cpu->regs.f, FLAG_Z_BIT)) {
				return 1;
			}
			break;
		case JP_C_IMM16:
		case CALL_C_IMM16:
		case JR_C_IMM8:
		case RET_C:
			if (BIT(cpu->regs.f, FLAG_C_BIT)) {
				return 1;
			}
			break;
		case JP_NZ_IMM16:
		case CALL_NZ_IMM16:
		case JR_NZ_IMM8:
		case RET_NZ:
			if (!BIT(cpu->regs.f, FLAG_Z_BIT)) {
				return 1;
			}
			break;
		case JP_NC_IMM16:
		case CALL_NC_IMM16:
		case JR_NC_IMM8:
		case RET_NC:
			if (!BIT(cpu->regs.f, FLAG_C_BIT)) {
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
	uint16_t pc = cpu->regs.pc;
	switch (cpu->op.kind) {
		case JP_IMM16:
		case CALL_IMM16:
			imm = next_imm16(cpu, cart);
			cpu->regs.pc = imm;
			break;
		case JP_ARR:
			assert(0);
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
			pc += (int8_t)next_imm8(cpu, cart);
			cpu->regs.pc = pc;
			break;
		case JR_Z_IMM8:
		case JR_C_IMM8:
		case JR_NZ_IMM8:
		case JR_NC_IMM8:
			imm = next_imm8(cpu, cart);
			if (flag_cond_met(cpu)) {
				pc += (int8_t)imm;
				cpu->regs.pc = pc;
			}
			break;
		case RETI:
			/*
			 * TODO: set interupt enabled flag
			 */
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
			imm = stack_pop(cpu, cart);
			if (flag_cond_met(cpu)) {
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
 * xor reg with itself, returns result for z flag (set bit on)
 */
static uint8_t op_xor_reg(Cpu *cpu)
{
	assert(XOR_R == cpu->op.kind);
	uint16_t z = read_reg(cpu, cpu->op.reg1);
	z ^= z;
	write_reg(cpu, cpu->op.reg1, z);
	return z;
}

static void op_xor(Cpu *cpu)
{
	uint8_t z = 0;
	switch (cpu->op.kind) {
		case XOR_R:
			z = op_xor_reg(cpu);
			break;
		case XOR_IMM8:
		case XOR_ARR:
			assert(0);
			break;
		default:
			assert(0);
			break;
	}
	set_flags(cpu, !z, 0, 0, 0);
}

static void op_ld(Cpu *cpu, Cartridge *cart)
{
	uint8_t h = 0;
	uint8_t c = 0;
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
			reg1 = read_reg(cpu, cpu->op.reg1);
			write_reg(cpu, cpu->op.reg1, reg1 + 1);
			break;
		case LD_ARRD_R:
			write_areg_reg(cpu, cart);
			reg1 = read_reg(cpu, cpu->op.reg1);
			write_reg(cpu, cpu->op.reg1, reg1 - 1);
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
			write_reg(cpu, cpu->op.reg2, reg2 + 1);
			break;
		case LD_R_ARRD:
			write_reg_areg(cpu, cart);
			write_reg(cpu, cpu->op.reg2, reg2 - 1);
			break;
		case LD_RR_RR_IMM8:
			imm = next_imm8(cpu, cart);
			reg2 = read_reg(cpu, cpu->op.reg2);
			write_reg(cpu, cpu->op.reg1, reg2 + (int8_t)imm);
			h = h_flag(reg2, reg2 + imm);
			c = c_flag(reg2, reg2 + imm);
			set_flags(cpu, 0, 0, h, c);
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

static void add_to_addr(Cartridge *cart, uint16_t addr, uint8_t val)
{
	uint8_t new_val = bus_read(cart, addr);
	new_val += val;
	bus_write8(cart, addr, new_val);
}

static void op_inc(Cpu *cpu, Cartridge *cart)
{
	uint16_t reg = read_reg(cpu, cpu->op.reg1);
	switch (cpu->op.kind) {
		case INC_R:
			write_reg(cpu, cpu->op.reg1, (uint8_t)(reg + 1));
			set_flags(cpu, z_flag(reg + 1), 0,
					h_flag(reg, reg + 1), 2);
			break;
		case INC_RR:
			write_reg(cpu, cpu->op.reg1, reg + 1);
			break;
		case INC_ARR:
			add_to_addr(cart, reg, 1);
			break;
		default:
			assert(0);
			break;
	}
}

static void op_dec(Cpu *cpu, Cartridge *cart)
{
	uint16_t reg = read_reg(cpu, cpu->op.reg1);
	switch (cpu->op.kind) {
		case DEC_R:
			write_reg(cpu, cpu->op.reg1, (uint8_t)(reg - 1));
			set_flags(cpu, z_flag(reg - 1), 1,
					h_flag(reg, reg - 1), 2);
			break;
		case DEC_RR:
			write_reg(cpu, cpu->op.reg1, reg - 1);
			break;
		case DEC_ARR:
			add_to_addr(cart, reg, -1);
			break;
		default:
			assert(0);
			break;
	}
}

static void op_push(Cpu *cpu, Cartridge *cart)
{
	Reg reg = cpu->op.reg1;
	assert(!is_8bit_reg(reg));
	uint16_t data = read_reg(cpu, cpu->op.reg1);
	stack_push(cpu, cart, data);
}

// used for POP, AF
static void set_pop_flags(Cpu *cpu, uint16_t data)
{
	/*
	 * these flags aren't correct yet
	 */
	 assert(0);
	uint8_t z = !data;
	uint8_t n = n_flag(data);
	uint8_t h = h_flag(0, data);
	uint8_t c = c_flag(0, data);
	set_flags(cpu, z, n, h, c);
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
static void op_pop(Cpu *cpu, const Cartridge *cart)
{
	Reg reg = cpu->op.reg1;
	assert(!is_8bit_reg(reg));
	uint16_t data = stack_pop(cpu, cart);
	write_reg(cpu, reg, data);

	if (REG_AF == cpu->op.reg1) {
		set_pop_flags(cpu, data);
	}
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
		case LD_RR_RR:
		case LDH_AIMM8_R:
		case LDH_R_AIMM8:
			op_ld(cpu, cart);
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
			op_inc(cpu, cart);
			break;
		case DEC_R:
		case DEC_RR:
		case DEC_ARR:
			op_dec(cpu, cart);
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
			stack_push(cpu, cart, cpu->regs.pc);
			op_jmp(cpu, cart);
			break;
		case PUSH_RR:
			op_push(cpu, cart);
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
	printf("a: 0x%02x ", regs.a);
	printf("f: 0x%02x ", regs.f);
	printf("b: 0x%02x ", regs.b);
	printf("c: 0x%02x\n", regs.c);
	printf("d: 0x%02x ", regs.d);
	printf("e: 0x%02x ", regs.e);
	printf("h: 0x%02x ", regs.h);
	printf("l: 0x%02x\n", regs.l);
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
