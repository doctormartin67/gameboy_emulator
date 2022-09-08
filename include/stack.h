#ifndef STACK_H
#define STACK_H
#include "cpu.h"
#include "cartridge.h"

void stack_push(Emulator *emu, uint16_t data);
uint16_t stack_pop(Emulator *emu);

#endif
