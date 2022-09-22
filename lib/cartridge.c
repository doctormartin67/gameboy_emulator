#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cartridge.h"

static const char *const cart_types[] = {
	[0x00] = "ROM ONLY",
	[0x01] = "MBC1",
	[0x02] = "MBC1+RAM",
	[0x03] = "MBC1+RAM+BATTERY",
	[0x05] = "MBC2",
	[0x06] = "MBC2+BATTERY",
	[0x08] = "ROM+RAM 1",
	[0x09] = "ROM+RAM+BATTERY 1",
	[0x0b] = "MMM01",
	[0x0c] = "MMM01+RAM",
	[0x0d] = "MMM01+RAM+BATTERY",
	[0x0f] = "MBC3+TIMER+BATTERY",
	[0x10] = "MBC3+TIMER+RAM+BATTERY 2",
	[0x11] = "MBC3",
	[0x12] = "MBC3+RAM 2",
	[0x13] = "MBC3+RAM+BATTERY 2",
	[0x19] = "MBC5",
	[0x1a] = "MBC5+RAM",
	[0x1b] = "MBC5+RAM+BATTERY",
	[0x1c] = "MBC5+RUMBLE",
	[0x1d] = "MBC5+RUMBLE+RAM",
	[0x1e] = "MBC5+RUMBLE+RAM+BATTERY",
	[0x20] = "MBC6",
	[0x22] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
	[0xfc] = "POCKET CAMERA",
	[0xfd] = "BANDAI TAMA5",
	[0xfe] = "HuC3",
	[0xff] = "HuC1+RAM+BATTERY",
};

static const char *const lic_codes[] = {
	[0x00] = "None",
	[0x01] = "Nintendo R&D1",
	[0x08] = "Capcom",
	[0x13] = "Electronic Arts",
	[0x18] = "Hudson Soft",
	[0x19] = "b-ai",
	[0x20] = "kss",
	[0x22] = "pow",
	[0x24] = "PCM Complete",
	[0x25] = "san-x",
	[0x28] = "Kemco Japan",
	[0x29] = "seta",
	[0x30] = "Viacom",
	[0x31] = "Nintendo",
	[0x32] = "Bandai",
	[0x33] = "Ocean/Acclaim",
	[0x34] = "Konami",
	[0x35] = "Hector",
	[0x37] = "Taito",
	[0x38] = "Hudson",
	[0x39] = "Banpresto",
	[0x41] = "Ubi Soft",
	[0x42] = "Atlus",
	[0x44] = "Malibu",
	[0x46] = "angel",
	[0x47] = "Bullet-Proof",
	[0x49] = "irem",
	[0x50] = "Absolute",
	[0x51] = "Acclaim",
	[0x52] = "Activision",
	[0x53] = "American sammy",
	[0x54] = "Konami",
	[0x55] = "Hi tech entertainment",
	[0x56] = "LJN",
	[0x57] = "Matchbox",
	[0x58] = "Mattel",
	[0x59] = "Milton Bradley",
	[0x60] = "Titus",
	[0x61] = "Virgin",
	[0x64] = "LucasArts",
	[0x67] = "Ocean",
	[0x69] = "Electronic Arts",
	[0x70] = "Infogrames",
	[0x71] = "Interplay",
	[0x72] = "Broderbund",
	[0x73] = "sculptured",
	[0x75] = "sci",
	[0x78] = "THQ",
	[0x79] = "Accolade",
	[0x80] = "misawa",
	[0x83] = "lozc",
	[0x86] = "Tokuma Shoten Intermedia",
	[0x87] = "Tsukuda Original",
	[0x91] = "Chunsoft",
	[0x92] = "Video system",
	[0x93] = "Ocean/Acclaim",
	[0x95] = "Varie",
	[0x96] = "Yonezawa/s’pal",
	[0x97] = "Kaneko",
	[0x99] = "Pack in soft",
	[0xa4] = "Konami (Yu-Gi-Oh!)",
};

static const char *get_lic_code(const Cartridge *cart)
{
	size_t size = sizeof(lic_codes) / sizeof(lic_codes[0]);
	uint16_t lic_code = cart->header->new_lic_code;
	if (lic_code > size || !lic_codes[lic_code]) {
		printf("Unknown licensee code");
		exit(1);
	}
	return lic_codes[lic_code];
}

static const char *get_cart_type(const Cartridge *cart)
{
	size_t size = sizeof(cart_types) / sizeof(cart_types[0]);
	uint8_t type = cart->header->type;
	if (type > size || !cart_types[type]) {
		printf("Unknown cartridge type");
		exit(1);
	}
	return cart_types[type];
}

Cartridge *cart_load(const char *file_name)
{
	FILE *fp = fopen(file_name, "r");
	if (!fp) {
		printf("Unable to open file '%s'.\n", file_name);
		exit(1);
	}
	if (fseek(fp, 0, SEEK_END)) {
		printf("Unexpected fseek error.\n");
		exit(1);
	}
	Cartridge *cart = malloc(sizeof(*cart));

	cart->rom_size = ftell(fp);	
	rewind(fp);

	cart->rom_data = calloc(cart->rom_size, sizeof(*cart->rom_data));
	if (1 != fread(cart->rom_data, cart->rom_size, 1, fp)) {
		printf("Unable to read data from file '%s'\n", file_name);
		exit(1);
	}
	cart->header = (struct cartridge_header *)(cart->rom_data + ENTR_ADDR);
	size_t title_size = sizeof(cart->header->title);
	cart->header->title[title_size - 1] = 0;
	
	cart->file_name = file_name;
	fclose(fp);
	return cart;
}

/*
 * returns false if checksum fails
 */
unsigned cart_check(const Cartridge *cart)
{
	assert(cart->header->checksum == cart->rom_data[CHECKSUM_ADDR]);
	printf("WARNING: NOT CHECKING CHECKSUM\n");
	return 1;
	uint8_t checksum = 0;
	for (uint16_t addr = TITLE_ADDR; addr <= MASK_ROM_V_N_ADDR; addr++) {
		checksum = checksum - cart->rom_data[addr] - 1;
	}

	if (checksum == cart->header->checksum) {
		return 1;
	} else {
		printf("checksum (file): %02x\n", cart->header->checksum);
		printf("checksum (calc): %02x\n", checksum);
		return 0;
	}
}

void cart_print(const Cartridge *cart)
{
	printf("Title: %s\n", cart->header->title);
	printf("Licensee code: %02x %s\n", cart->header->new_lic_code,
			get_lic_code(cart));
	printf("Cartridge type: %02x %s\n", cart->header->type,
			get_cart_type(cart));
	printf("ROM size: %02d KB\n", 32 * (1 << cart->header->rom_size));
	printf("RAM size: %02x\n", cart->header->ram_size);
	printf("Mask ROM version number: %02x\n",
			cart->header->mask_ROM_version_number);
	printf("checksum: %02x\n", cart->header->checksum);
}

uint8_t cart_read(const Cartridge *cart, uint16_t addr)
{
	assert(addr < cart->rom_size);
	return cart->rom_data[addr];
}

void cart_write(Cartridge *cart, uint16_t addr, uint8_t data)
{
	assert(addr < cart->rom_size);
	(void)data;
	assert(0);
}

Cartridge *cart_init(const char *file_name)
{
	Cartridge *cart = cart_load(file_name);
	assert(cart_check(cart));
	return cart;
}

void free_cart(Cartridge *cart)
{
	free(cart->rom_data);
	free(cart);
}
