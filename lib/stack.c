#include "bus.h"
#include "stack.h"
#include "common.h"

static void stack_push8(Emulator *emu, uint8_t data)
{
	emu->cpu->regs.sp--;
	bus_write8(emu, emu->cpu->regs.sp, data);
}

void stack_push(Emulator *emu, uint16_t data)
{
	stack_push8(emu, LO_SHIFT(data) & 0xff);
	stack_push8(emu, data & 0xff);
}

static uint8_t stack_pop8(Emulator *emu)
{
	return bus_read(emu, emu->cpu->regs.sp++);
}

uint16_t stack_pop(Emulator *emu)
{
	uint16_t lo = stack_pop8(emu);
	uint16_t hi = stack_pop8(emu);
	return HI_SHIFT(hi) | lo;
}
