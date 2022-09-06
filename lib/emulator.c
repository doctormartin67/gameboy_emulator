#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "emulator.h"
#include "ui.h"
#include "transfer.h"

Emulator *emu_init(Cpu *cpu, Cartridge *cart)
{
	Emulator *emu = malloc(sizeof(*emu));
	*emu = (Emulator){
		.running = 1, .playing = 0, .ticks = 0,
			.cpu = cpu, .cart = cart};
	return emu;
}

void emu_kill(Emulator *emu)
{
	emu->running = 0;	
}

void *cpu_run(void *p)
{
	Emulator *emu = p;
	while (emu->running) {
		//if (!(emu->ticks % 10)) (void)getchar();
		printf("%9lu ", emu->ticks);
		cpu_print(emu->cpu, emu->cart);
		next_op(emu->cpu, emu->cart);
		update_transfer_msg(emu->cart);
		print_transfer_msg();
		emu->ticks++;
	}
	return 0;
}

int emu_main(int argc, char *argv[])
{
	if (2 != argc) {
		printf("Usage: ./main 'file name'\n");
		return 1;
	}

	Cartridge *cart = cart_init(argv[1]);
	Cpu *cpu = cpu_init();
	Emulator *emu = emu_init(cpu, cart);
	ui_init();

	cart_print(cart);

	pthread_t cpu_thread;

	if (pthread_create(&cpu_thread, 0, cpu_run, emu)) {
		printf("Unable to start cpu thread\n");
		return 1;
	}

	while(emu->running) {
		ui_handle_events(emu);
	}
	return 0;
}

