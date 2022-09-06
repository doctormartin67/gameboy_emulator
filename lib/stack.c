#include "bus.h"
#include "stack.h"
#include "common.h"

static void stack_push8(Cpu *cpu, Cartridge *cart, uint8_t data)
{
	cpu->regs.sp--;
	bus_write8(cart, cpu->regs.sp, data);
}

void stack_push(Cpu *cpu, Cartridge *cart, uint16_t data)
{
	stack_push8(cpu, cart, LO_SHIFT(data) & 0xff);
	stack_push8(cpu, cart, data & 0xff);
}

static uint8_t stack_pop8(Cpu *cpu, const Cartridge *cart)
{
	return bus_read(cart, cpu->regs.sp++);
}

uint16_t stack_pop(Cpu *cpu, const Cartridge *cart)
{
	uint16_t lo = stack_pop8(cpu, cart);
	uint16_t hi = stack_pop8(cpu, cart);
	return HI_SHIFT(hi) | lo;
}
