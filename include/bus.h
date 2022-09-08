#ifndef BUS_H
#define BUS_H

#include "emulator.h"

uint8_t bus_read(const Emulator *emu, uint16_t addr);
void bus_write8(Emulator *emu, uint16_t addr, uint8_t data);
void bus_write16(Emulator *emu, uint16_t addr, uint16_t data);

#endif
