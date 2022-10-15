#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

#define SOUND_ADDR 0xff10
#define IS_SOUND_ADDR(a) ((a) < LCD_ADDR && (a) >= SOUND_ADDR)

// https://gbdev.io/pandocs/Sound_Controller.html#sound-controller

struct sound {
	uint8_t nr1[4];
	uint8_t garbage1; // 0xff15
	uint8_t nr2[4];
	uint8_t nr3[4];
	uint8_t garbage2; // 0xff1f
	uint8_t nr4[4];
	uint8_t nr5[4];
	uint8_t garbage[9];
	uint8_t wave[16];
};

uint8_t sound_read(const struct sound *s, uint16_t addr);
void sound_write(struct sound *s, uint16_t addr, uint8_t data);

#endif
