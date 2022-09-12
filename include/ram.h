#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define RAM_ADDR 0xc000
#define HRAM_ADDR 0xff80
#define WRAM_SIZE 0x2000
#define HRAM_SIZE 0x80

typedef struct Ram {
	uint8_t wram[WRAM_SIZE];
	uint8_t hram[HRAM_SIZE];
} Ram;

uint8_t wram_read(uint16_t addr);
void wram_write(uint16_t addr, uint8_t data);

uint8_t hram_read(uint16_t addr);
void hram_write(uint16_t addr, uint8_t data);

#endif
