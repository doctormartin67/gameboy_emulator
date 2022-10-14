#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdint.h>
// https://gbdev.io/pandocs/Joypad_Input.html

/*
Bit 7 - Not used
Bit 6 - Not used
Bit 5 - P15 Select Action buttons    (0=Select)
Bit 4 - P14 Select Direction buttons (0=Select)
Bit 3 - P13 Input: Down  or Start    (0=Pressed) (Read Only)
Bit 2 - P12 Input: Up    or Select   (0=Pressed) (Read Only)
Bit 1 - P11 Input: Left  or B        (0=Pressed) (Read Only)
Bit 0 - P10 Input: Right or A        (0=Pressed) (Read Only)
*/

struct joypad {
	unsigned action;
	unsigned direction;
	unsigned a;
	unsigned b;
	unsigned select;
	unsigned start;
	unsigned up;
	unsigned down;
	unsigned left;
	unsigned right;
};

typedef enum {
	JOYP_DIRECTION = 4,
	JOYP_ACTION,
} ButtonKind;

uint8_t joypad_read(const struct joypad *jp);
void joypad_write(struct joypad *jp, uint8_t data);

#endif
