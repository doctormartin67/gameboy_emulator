#include <stdlib.h>
#include <assert.h>
#include "timer.h"
#include "common.h"

Timer *timer_init(void)
{
	Timer *t = malloc(sizeof(*t));	
#if 0
	t->clock = 0;
	t->div_tima = 0xab00;
	t->tma = 0x00;
	t->tac = 0xf8;
#else
	t->clock = 0xabcc;
	t->div_tima = 0xab00;
	t->tma = 0x00;
	t->tac = 0x00;
#endif
	return t;
}

void timer_tick(Cpu *cpu, Timer *t)
{
	unsigned update_tima = 0;
	uint16_t c = t->clock;

	switch (t->tac & 0x03) {
		case 0x00:
			update_tima = BIT(c, 9) && !BIT(c + 1, 9);
			break;
		case 0x01:
			update_tima = BIT(c, 3) && !BIT(c + 1, 3);
			break;
		case 0x02:
			update_tima = BIT(c, 5) && !BIT(c + 1, 5);
			break;
		case 0x03:
			update_tima = BIT(c, 7) && !BIT(c + 1, 7);
			break;
		default:
			assert(0);
			break;
	}

	if (update_tima && BIT(t->tac, 2)) {
		t->div_tima++;
		if ((t->div_tima & 0xff) == 0xff) {
			t->div_tima = (t->div_tima & 0xff00) | t->tma;
			cpu_request_interrupt(cpu, INT_TIMER);
		}
	}
	
	t->div_tima = (++t->clock & 0xff00) | (t->div_tima & 0x00ff);
}

uint8_t timer_read(const Timer *t, uint16_t addr)
{
	switch (addr) {
		case DIV_ADDR:
			return t->div_tima >> 8;
		case TIMA_ADDR:
			return t->div_tima;
		case TMA_ADDR:
			return t->tma;
		case TAC_ADDR:
			return t->tac;
		default:
			assert(0);
			break;
	}
}

void timer_write(Timer *t, uint16_t addr, uint8_t data)
{
	switch (addr) {
		case DIV_ADDR:
			t->div_tima &= 0x00ff;
			t->clock = 0;
			return;
		case TIMA_ADDR:
			t->div_tima &= 0xff00;
			t->div_tima |= data;
			return;
		case TMA_ADDR:
			t->tma = data;
			return;
		case TAC_ADDR:
			t->tac = data;
			return;
		default:
			assert(0);
			break;
	}
}
