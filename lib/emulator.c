#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "emulator.h"
#include "ui.h"
#include "transfer.h"
#include "interrupts.h"

Emulator *emu_init(Cpu *cpu, Cartridge *cart, Timer *timer)
{
	Emulator *emu = malloc(sizeof(*emu));
	*emu = (Emulator){
		.running = 1, .playing = 0, .ticks = 0,
			.cpu = cpu, .cart = cart, .timer = timer};
	return emu;
}

void emu_kill(Emulator *emu)
{
	emu->running = 0;
}

void emu_ticks(Emulator *emu, unsigned ticks)
{
	for (unsigned i = 0; i < ticks; i++) {
		emu->ticks++;
		timer_tick(emu->cpu, emu->timer);
	}
}

void *cpu_run(void *p)
{
	Emulator *emu = p;
	while (emu->running) {
		//if (!(emu->ticks % 32)) (void)getchar();
		if (!emu->cpu->halted) {
			printf("%09lx ", emu->ticks);
			cpu_print(emu);
			next_op(emu);
			update_transfer_msg(emu);
			print_transfer_msg();
		} else {
			// TODO: set halted to false if flags?
			emu_ticks(emu, 4);
		}
		cpu_int_handler(emu);
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
	Timer *timer = timer_init();
	Emulator *emu = emu_init(cpu, cart, timer);
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
