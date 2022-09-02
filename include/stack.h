#ifndef STACK_H
#define STACK_H
#include <stdint.h>
#include "cpu.h"
#include "bus.h"
#include "cartridge.h"

void stack_push(Cpu *cpu, Cartridge *cart, uint16_t data);
uint16_t stack_pop(Cpu *cpu, const Cartridge *cart);

#endif
