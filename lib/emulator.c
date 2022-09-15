#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "emulator.h"
#include "ui.h"
#include "interrupts.h"

Emulator *emu_init(Cpu *cpu, Cartridge *cart, Timer *timer, Ppu *ppu)
{
	Emulator *emu = malloc(sizeof(*emu));
	*emu = (Emulator){
		.running = 1, .playing = 0, .ticks = 0,
			.cpu = cpu, .cart = cart, .timer = timer, .ppu = ppu};
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
		//if (emu->ticks >= 0x1a9dc8) (void)getchar();
		//if (!(emu->ticks % 2048)) (void)getchar();
		if (!emu->cpu->halted) {
			print_status(emu);
			next_op(emu);
		} else {
			emu_ticks(emu, 4);
			/* 
			 * TODO: understand the below code, why should this
			 * set it to false again?
			 */
			if (emu->cpu->if_reg) {
				emu->cpu->halted = 0;
			}
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
	Ppu *ppu = ppu_init();
	Emulator *emu = emu_init(cpu, cart, timer, ppu);
	ui_init();

	cart_print(cart);

	pthread_t cpu_thread;

	if (pthread_create(&cpu_thread, 0, cpu_run, emu)) {
		printf("Unable to start cpu thread\n");
		return 1;
	}

	while(emu->running) {
		ui_handle_events(emu);
		ui_update(emu);
	}
	return 0;
}
