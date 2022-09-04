#ifndef EMULATOR_H
#define EMULATOR_H

typedef struct Emulator {
	unsigned running;
	unsigned paused;
} Emulator;

int emu_main(int argc, char *argv[]);
Emulator *emu_init(void);

#endif
