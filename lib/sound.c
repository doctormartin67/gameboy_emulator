#include "sound.h"

uint8_t sound_read(const struct sound *s, uint16_t addr)
{
	addr -= SOUND_ADDR;
	return ((uint8_t *)s)[addr];

}

void sound_write(struct sound *s, uint16_t addr, uint8_t data)
{
	addr -= SOUND_ADDR;
	((uint8_t *)s)[addr] = data;
}
