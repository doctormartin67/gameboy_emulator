#ifndef IO_H
#define IO_H

#include <stdint.h>

#define SB_ADDR 0xff01
#define SC_ADDR 0xff02

uint8_t io_read(uint16_t addr);
void io_write(uint16_t addr, uint8_t data);

#endif
