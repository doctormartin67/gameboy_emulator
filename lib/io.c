#include <stdio.h>
#include <assert.h>
#include "io.h"
#include "cpu.h"
#include "lcd.h"

// https://gbdev.io/pandocs/Serial_Data_Transfer_(Link_Cable).html

static uint8_t serial_data[2];

uint8_t io_read(const Emulator *emu, uint16_t addr)
{
	if (SB_ADDR == addr) {
		return serial_data[0];
	} else if (SC_ADDR == addr) {
		return serial_data[1];
	} else if (TIMER_ADDR(addr)) {
		return timer_read(emu->timer, addr);
	} else if (IF_ADDR == addr) {
		return cpu_if_reg_read(emu->cpu);
	} else if (IS_LCD_ADDR(addr)) {
		return lcd_read(emu->ppu->lcd, addr);
	}
	printf("Read at address '0x%04x' not supported yet\n", addr);
	return 0;
}

void io_write(Emulator *emu, uint16_t addr, uint8_t data)
{
	if (SB_ADDR == addr) {
		serial_data[0] = data;
		return;
	} else if (SC_ADDR == addr) {
		serial_data[1] = data;
		return;
	} else if (TIMER_ADDR(addr)) {
		timer_write(emu->timer, addr, data);
		return;
	} else if (IF_ADDR == addr) {
		emu->cpu->if_reg = data;
		return;
	} else if (IS_LCD_ADDR(addr)) {
		lcd_write(emu->ppu, addr, data);
		return;
	}
	printf("Write at address '0x%04x' not supported yet\n", addr);
}
