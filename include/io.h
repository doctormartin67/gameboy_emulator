#ifndef IO_H
#define IO_H

#include <stdint.h>
#include "cpu.h"

#define SB_ADDR 0xff01
#define SC_ADDR 0xff02
#define TIMER_ADDR(a) ((a) < 0xff08 && (a) > 0xff03)
#define IF_ADDR 0xff0f

uint8_t io_read(const Cpu *cpu, uint16_t addr);
void io_write(Cpu *cpu, uint16_t addr, uint8_t data);

#endif
