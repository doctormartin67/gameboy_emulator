#ifndef TRANSFER_H
#define TRANSFER_H

#include "emulator.h"

// https://gbdev.io/pandocs/Serial_Data_Transfer_(Link_Cable).html
#define TRANSFER_SIZE 1024
#define TRANSFER_REQUESTED 0x81
#define TRANSFER_ACCEPTED 0

void update_transfer_msg(Emulator *emu);
void print_transfer_msg(void);

#endif
