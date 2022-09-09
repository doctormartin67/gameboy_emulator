#include <stdio.h>
#include <assert.h>
#include "bus.h"
#include "ram.h"
#include "io.h"
#include "common.h"

// file:///home/doctormartin67/Downloads/The%20Cycle-Accurate%20Game%20Boy%20Docs.pdf
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

uint8_t bus_read(const Emulator *emu, uint16_t addr)
{
	if (addr < 0x8000) {
		return cart_read(emu->cart, addr);
	} else if (addr < 0xa000) {
		printf("Read at address '0x%04x' not supported yet\n", addr);
		return 0;
	} else if (addr < 0xc000) {
		return cart_read(emu->cart, addr);
	} else if (addr < 0xe000) {
		return wram_read(addr);
	} else if (addr < 0xfe00) {
		printf("Read at address '0x%04x' not supported yet\n", addr);
		return 0;
	} else if (addr < 0xfea0) {
		printf("Read at address '0x%04x' not supported yet\n", addr);
		return 0;
	} else if (addr < 0xff00) {
		printf("Read at address '0x%04x' not supported yet\n", addr);
		return 0;
	} else if (addr < 0xff80) {
		return io_read(emu, addr);
	} else if (0xffff == addr) {
		return cpu_ie_reg_read(emu->cpu);
	} else {
		return hram_read(addr);
	}
}

void bus_write8(Emulator *emu, uint16_t addr, uint8_t data)
{
	if (addr < 0x8000) {
		printf("ERROR: Trying to write to ROM\n");
	} else if (addr < 0xa000) {
		printf("Write at address '0x%04x' not supported yet\n", addr);
	} else if (addr < 0xc000) {
		cart_write(emu->cart, addr, data);
	} else if (addr < 0xe000) {
		wram_write(addr, data);
	} else if (addr < 0xfe00) {
		printf("Write at address '0x%04x' not supported yet\n", addr);
	} else if (addr < 0xfea0) {
		printf("Write at address '0x%04x' not supported yet\n", addr);
	} else if (addr < 0xff00) {
		printf("Write at address '0x%04x' not supported yet\n", addr);
	} else if (addr < 0xff80) {
		io_write(emu, addr, data);
	} else if (addr == 0xffff) {
		cpu_ie_reg_write(emu->cpu, data);
	} else {
		hram_write(addr, data);
	}
}

void bus_write16(Emulator *emu, uint16_t addr, uint16_t data)
{
	bus_write8(emu, addr, data & 0xff);
	bus_write8(emu, addr + 1, LO_SHIFT(data) & 0xff);
}
