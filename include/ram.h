#ifndef RAM_H
#define RAM_H

#include <stdint.h>

typedef struct Ram {
	uint8_t wram[0x2000];
	uint8_t hram[0x80];
} Ram;

uint8_t wram_read(uint16_t addr);
void wram_write(uint16_t addr, uint8_t data);

uint8_t hram_read(uint16_t addr);
void hram_write(uint16_t addr, uint8_t data);

#endif
