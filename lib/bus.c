#include <assert.h>
#include "bus.h"

uint8_t bus_read(const Cartridge *cart, uint16_t addr)
{
	assert(addr < cart->rom_size);
	return cart_read(cart, addr);
}

void bus_write(Cartridge *cart, uint16_t addr, uint8_t data)
{
	assert(addr < cart->rom_size);
	cart_write(cart, addr, data);
}
