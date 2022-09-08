#include <assert.h>
#include "interrupts.h"
#include "stack.h"

static void int_handler(Emulator *emu, uint16_t addr)
{
	stack_push(emu->cpu, emu->cart, emu->cpu->regs.pc);
	emu->cpu->regs.pc = addr;
}

/*
 * return 1 if interrupt handled, 0 otherwise
 */
static unsigned handle_int(Emulator *emu, enum interrupt it, uint16_t addr)
{
	assert(emu->cpu->ime_flag);
	if (emu->cpu->ie_reg & it && emu->cpu->if_reg & it) {
		int_handler(emu, addr);
		emu->cpu->ime_flag = 0;
		emu->cpu->if_reg = 0;
		return 1;
	}
	return 0;
}

void cpu_int_handler(Emulator *emu)
{
	assert(emu->cpu->ime_flag);
	assert(emu->cpu->ie_reg);
	assert(emu->cpu->if_reg);
	if (handle_int(emu, INT_VBLANK, 0x40)) {
		return;
	} else if (handle_int(emu, INT_LCD_STAT, 0x48)) {
		return;
	} else if (handle_int(emu, INT_TIMER, 0x50)) {
		return;
	} else if (handle_int(emu, INT_SERIAL, 0x58)) {
		return;
	} else if (handle_int(emu, INT_JOYPAD, 0x60)) {
		return;
	} else {
		/* 
		 * not sure yet if this function can be called with no
		 * interrupt waiting
		 */
		assert(0);
	}
}
