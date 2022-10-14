#include <assert.h>
#include "joypad.h"
#include "common.h"
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

static uint8_t joypad_read_action(const struct joypad *jp)
{
	uint8_t input = 0xcf; // 3 bottom bits on means not pressed

	if (jp->a) {
		input &= ~0x01;
	}
	if (jp->b) {
		input &= ~0x02;
	}
	if (jp->select) {
		input &= ~0x04;
	}
	if (jp->start) {
		input &= ~0x08;
	}
	return input;
}

static uint8_t joypad_read_direction(const struct joypad *jp)
{
	uint8_t input = 0xcf; // 3 bottom bits on means not pressed

	if (jp->right) {
		input &= ~0x01;
	}
	if (jp->left) {
		input &= ~0x02;
	}
	if (jp->up) {
		input &= ~0x04;
	}
	if (jp->down) {
		input &= ~0x08;
	}
	return input;
}

uint8_t joypad_read(const struct joypad *jp)
{
	if (jp->action) {
		return joypad_read_action(jp);
	}
	assert(jp->direction);
	return joypad_read_direction(jp);
}

void joypad_write(struct joypad *jp, uint8_t data)
{
	jp->action = !(data & (1 << JOYP_ACTION));
	jp->direction = !(data & (1 << JOYP_DIRECTION));
}
