#include <stdio.h>
#include <assert.h>
#include "io.h"

// https://gbdev.io/pandocs/Serial_Data_Transfer_(Link_Cable).html

static uint8_t serial_data[2];

uint8_t io_read(uint16_t addr)
{
	if (SB_ADDR == addr) {
		return serial_data[0];
	} else if (SC_ADDR == addr) {
		return serial_data[1];
	}
	printf("Read at address '0x%04x' not supported yet\n", addr);
	return 0;
}

void io_write(uint16_t addr, uint8_t data)
{
	if (SB_ADDR == addr) {
		assert(0);
		serial_data[0] = data;
		return;
	} else if (SC_ADDR == addr) {
		serial_data[1] = data;
		return;
	}
	printf("Write at address '0x%04x' not supported yet\n", addr);
}
