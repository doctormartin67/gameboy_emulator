#include <stdio.h>
#include <assert.h>
#include "bus.h"
#include "ram.h"
#include "io.h"
#include "ppu.h"
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
	if (addr < VRAM_ADDR) {
		return cart_read(emu->cart, addr);
	} else if (addr < 0xa000) {
		return ppu_vram_read(emu->ppu, addr);
	} else if (addr < RAM_ADDR) {
		return cart_read(emu->cart, addr);
	} else if (addr < 0xe000) {
		return wram_read(addr);
	} else if (addr < OAM_ADDR) {
		printf("Read at address '0x%04x' not supported yet\n", addr);
		return 0;
	} else if (addr < 0xfea0) {
		if (dma_transferring(emu->ppu->dma)) {
			return 0xff;	
		}
		return ppu_oam_read(emu->ppu, addr);
	} else if (addr < IO_ADDR) {
		printf("Read at address '0x%04x' not supported yet\n", addr);
		return 0;
	} else if (addr < HRAM_ADDR) {
		return io_read(emu, addr);
	} else if (0xffff == addr) {
		return cpu_ie_reg_read(emu->cpu);
	} else {
		return hram_read(addr);
	}
}

extern unsigned hack;
void bus_write8(Emulator *emu, uint16_t addr, uint8_t data)
{
	if (addr < VRAM_ADDR) {
		printf("ERROR: Trying to write to ROM\n");
	} else if (addr < 0xa000) {
		ppu_vram_write(emu->ppu, addr, data);
	} else if (addr < RAM_ADDR) {
		cart_write(emu->cart, addr, data);
	} else if (addr < 0xe000) {
		wram_write(addr, data);
	} else if (addr < OAM_ADDR) {
		printf("Write at address '0x%04x' not supported yet\n", addr);
	} else if (addr < 0xfea0) {
		if (dma_transferring(emu->ppu->dma)) {
			return;
		}
		ppu_oam_write(emu->ppu, addr, data);
	} else if (addr < IO_ADDR) {
		printf("Write at address '0x%04x' not supported yet\n", addr);
	} else if (addr < HRAM_ADDR) {
		io_write(emu, addr, data);
	} else if (addr == 0xffff) {
		cpu_ie_reg_write(emu->cpu, data);
	} else {
		hram_write(addr, data);
	}
	emu_ticks(emu, TICKS_PER_CYCLE);
}

void bus_write16(Emulator *emu, uint16_t addr, uint16_t data)
{
	bus_write8(emu, addr, data & 0xff);
	bus_write8(emu, addr + 1, LO_SHIFT(data) & 0xff);
}
