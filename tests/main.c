#include <stdio.h>
#include <stdlib.h>
#include "cartridge.h"
#include "cpu.h"

int main(int argc, char *argv[])
{
	if (2 != argc) {
		printf("Usage: ./a.out 'file name'\n");
		exit(1);
	}

	Cartridge *cart = cart_load(argv[1]);
	if (!cart_check(cart)) {
		printf("checksum is incorrect\n");
		exit(1);
	}
	cart_print(cart);

	Cpu cpu = {.regs = {.pc = 0x100}};
	while (1) {
		(void)getchar();
		cpu_print(&cpu, cart);
		next_op(&cpu, cart);
	}
	return 0;
}
