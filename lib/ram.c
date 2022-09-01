#include <assert.h>
#include "ram.h"

/*
   0000h – 3FFFh ROM0 Non-switchable ROM Bank.
   4000h – 7FFFh ROMX Switchable ROM bank.
   8000h – 9FFFh VRAM Video RAM, switchable (0-1) in GBC mode.
   A000h – BFFFh SRAM External RAM in cartridge, often battery buffered.
   C000h – CFFFh WRAM0 Work RAM.
   D000h – DFFFh WRAMX Work RAM, switchable (1-7) in GBC mode
   E000h – FDFFh ECHO Description of the behaviour below.
   FE00h – FE9Fh OAM (Object Attribute Table) Sprite information table.
   FEA0h – FEFFh UNUSED Description of the behaviour below.
   FF00h – FF7Fh I/O Registers I/O registers are mapped here.
   FF80h – FFFEh HRAM Internal CPU RAM
   FFFFh IE Register Interrupt enable flags.
 */

/*
 * for now I made this a global, if I ever wanted more than one emulators
 * at the same time I would need to adjust the code everywhere
 */

static Ram ram;

uint8_t wram_read(uint16_t addr)
{
	addr -= 0xc000;
	assert(addr < sizeof(ram.wram));
	return ram.wram[addr];
}

void wram_write(uint16_t addr, uint8_t data)
{
	addr -= 0xc000;
	assert(addr < sizeof(ram.wram));
	ram.wram[addr] = data;
}

uint8_t hram_read(uint16_t addr)
{
	addr -= 0xff80;
	assert(addr < sizeof(ram.hram));
	return ram.hram[addr];
}

void hram_write(uint16_t addr, uint8_t data)
{
	addr -= 0xff80;
	assert(addr < sizeof(ram.hram));
	ram.hram[addr] = data;
}
