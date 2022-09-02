#ifndef BUS_H
#define BUS_H

#include "cartridge.h"

uint8_t bus_read(const Cartridge *cart, uint16_t addr);
void bus_write8(Cartridge *cart, uint16_t addr, uint8_t data);
void bus_write16(Cartridge *cart, uint16_t addr, uint16_t data);

#endif
