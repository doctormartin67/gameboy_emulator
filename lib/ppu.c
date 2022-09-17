#include <stdlib.h>
#include <assert.h>
#include "ppu.h"
#include "common.h"

Ppu *ppu_init(void)
{
	Ppu *ppu = malloc(sizeof(*ppu));
	*ppu = (Ppu){0};
	ppu->lcd = lcd_init();
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

// https://gbdev.io/pandocs/Palettes.html

static void update_palettes(uint32_t *pal, uint8_t color)
{
	for (size_t i = 0; i < 4; i++) {
		pal[i] = default_colors[(color >> (2 * i)) & 0x3];
	}
}

void lcd_write(Ppu *ppu, uint16_t addr, uint8_t data)
{
	Lcd *lcd = ppu->lcd;
	assert(lcd);
	assert(addr >= LCD_ADDR);
	uint8_t offset = addr - LCD_ADDR;
	((uint8_t *)lcd)[offset] = data;

	if (DMA_ADDR == addr) {
		dma_start(ppu, data);
	} else if (BGP_ADDR == addr) {
		update_palettes(lcd->bg_colors, data);
	} else if (OBP1_ADDR == addr) {
		// 0xfc is used to mask out bottom 2 bits
		update_palettes(lcd->sprite1_colors, data & 0xfc);
	} else if (OBP2_ADDR == addr) {
		update_palettes(lcd->sprite2_colors, data & 0xfc);
	}
}
