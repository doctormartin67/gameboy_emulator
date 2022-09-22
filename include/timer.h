#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "cpu.h"

// https://gbdev.io/pandocs/Timer_and_Divider_Registers.html

#define DIV_ADDR 0xff04
#define TIMA_ADDR 0xff05
#define TMA_ADDR 0xff06
#define TAC_ADDR 0xff07

typedef struct Timer {
	uint16_t clock;
	uint16_t div_tima;
	uint8_t tma;
	uint8_t tac;
} Timer;

// https://gbdev.io/pandocs/Power_Up_Sequence.html?highlight=div#hardware-registers

Timer *timer_init(void);
void free_timer(Timer *timer);
void timer_tick(Cpu *cpu, Timer *t);
uint8_t timer_read(const Timer *t, uint16_t addr);
void timer_write(Timer *t, uint16_t addr, uint8_t data);

#endif
