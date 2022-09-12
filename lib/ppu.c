#include <stdlib.h>
#include <assert.h>
#include "ppu.h"

Ppu *ppu_init(void)
{
	Ppu *ppu = malloc(sizeof(*ppu));
	*ppu = (Ppu){0};
	return ppu;
}

void ppu_tick(Ppu *ppu)
{
	(void)ppu;
}

/*
 * the assertions made for oam might not always hold. If they don't, it might
 * be the case that you don't use the offset and thus just use the addr as is.
 */
uint8_t ppu_oam_read(const Ppu *ppu, uint16_t addr)
{
	addr -= OAM_ADDR;
	assert(addr < sizeof(ppu->oam));
	return ((uint8_t *)(ppu->oam))[addr];
}

void ppu_oam_write(Ppu *ppu, uint16_t addr, uint8_t data)
{
	addr -= OAM_ADDR;
	assert(addr < sizeof(ppu->oam));
	((uint8_t *)(ppu->oam))[addr] = data;
}
uint8_t ppu_vram_read(const Ppu *ppu, uint16_t addr)
{
	addr -= VRAM_ADDR;
	assert(addr < VRAM_SIZE);
	return ppu->vram[addr];

}
void ppu_vram_write(Ppu *ppu, uint16_t addr, uint8_t data)
{
	addr -= VRAM_ADDR;
	assert(addr < VRAM_SIZE);
	ppu->vram[addr] = data;
}
