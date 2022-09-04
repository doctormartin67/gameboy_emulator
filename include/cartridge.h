#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdlib.h>
#include <stdint.h>

// https://gbdev.io/pandocs/The_Cartridge_Header.html

#define ENTR_ADDR 0x100
#define TITLE_ADDR 0x0134
#define MASK_ROM_V_N_ADDR 0x014c
#define CHECKSUM_ADDR 0x014d

/*
 * Note: this struct is perfectly alligned so that we can cast the rom data
 * to this struct type pointer (size == 80).
 */
struct cartridge_header {
	uint32_t entry; // 0x0100 - 0x0103
	uint8_t logo[0x133 - 0x104 + 1]; // 0x0104 - 0x0133
	uint8_t title[0x143 - 0x134 + 1]; // 0x0134 - 0x0143
	uint16_t new_lic_code; // 0x0144 - 0x0145
	uint8_t SGB_flag; // 0x0146
	uint8_t type; // 0x0147
	uint8_t rom_size; // 0x0148
	uint8_t ram_size; // 0x0149
	uint8_t dest_code; // 0x014a
	uint8_t old_lic_code; // 0x014b
	uint8_t mask_ROM_version_number; // 0x014c
	uint8_t checksum; // 0x014d
	uint16_t global_check_sum; // 0x014e - 0x014f
};

typedef struct Cartridge {
	struct cartridge_header *header;
	size_t rom_size;
	uint8_t *rom_data;
	const char *file_name;
} Cartridge;

Cartridge *cart_load(const char *file_name);
unsigned cart_check(const Cartridge *cart);
void cart_print(const Cartridge *cart);
uint8_t cart_read(const Cartridge *cart, uint16_t addr);
void cart_write(Cartridge *cart, uint16_t addr, uint8_t data);
Cartridge *cart_init(const char *file_name);

#endif
