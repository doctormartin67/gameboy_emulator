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
	if ((unsigned long)(addr - OAM_ADDR) < sizeof(ppu->oam)) {
		addr -= OAM_ADDR;
	} 
	assert(addr < sizeof(ppu->oam));
	return ((uint8_t *)(ppu->oam))[addr];
}

void ppu_oam_write(Ppu *ppu, uint16_t addr, uint8_t data)
{
	if ((unsigned long)(addr - OAM_ADDR) < sizeof(ppu->oam)) {
		addr -= OAM_ADDR;
	} 
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

void dma_start(Ppu *ppu, uint8_t start)
{
	if (!ppu->dma) {
		ppu->dma = malloc(sizeof(*ppu->dma));
	}
	ppu->dma->transferring = 1;
	ppu->dma->byte = 0;
	ppu->dma->delay = 2;
	ppu->dma->value = start;
}

