#include <stdio.h>
#include <assert.h>
#include "transfer.h"
#include "bus.h"
#include "io.h"

static char msg[TRANSFER_SIZE];
static size_t msg_size;

void update_transfer_msg(Emulator *emu)
{
	if (bus_read(emu, SC_ADDR) == TRANSFER_REQUESTED) {
		assert(msg_size < TRANSFER_SIZE);
		msg[msg_size++] = bus_read(emu, SB_ADDR);
		bus_write8(emu, SC_ADDR, TRANSFER_ACCEPTED);
	}
}

void print_transfer_msg(void)
{
	if (msg[0]) printf("Transfer message: %s\n", msg);
}
