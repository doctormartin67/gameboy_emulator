#include <stdio.h>
#include <stdlib.h>
#include "emulator.h"
#include "cartridge.h"
#include "cpu.h"

Emulator *emu_init(void)
{
	Emulator *emu = malloc(sizeof(*emu));
	*emu = (Emulator){.running = 1, .paused = 0};
	return emu;
}

int emu_main(int argc, char *argv[])
{
	if (2 != argc) {
		printf("Usage: ./main 'file name'\n");
		return 1;
	}

	Cartridge *cart = cart_init(argv[1]);
	Cpu *cpu = cpu_init();
	Emulator *emu = emu_init();

	cart_print(cart);

	while (emu->running) {
		//(void)getchar();
		cpu_print(cpu, cart);
		next_op(cpu, cart);
	}
	return 0;
}

