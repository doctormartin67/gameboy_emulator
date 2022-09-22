#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include "ppu.h"
#include "common.h"

Ppu *ppu_init(void)
{
	Ppu *ppu = calloc(1, sizeof(*ppu));
	ppu->lcd = lcd_init();
	return ppu;
}

void free_ppu(Ppu *ppu)
{
	free(ppu->lcd);
	free(ppu);
}

// https://gbdev.io/pandocs/pixel_fifo.html
static void increment_ly(Cpu *cpu, Lcd *lcd) {
	lcd->ly++;
	set_ly_flag(cpu, lcd);
}

static void update_fps(void)
{
	static double start_frame_time = 0;
	double end_frame_time = get_ticks();
	double frame_time = end_frame_time - start_frame_time;

	if (frame_time < TARGET_FRAME_TIME) {
		delay(TARGET_FRAME_TIME - frame_time);
	}
	start_frame_time = get_ticks();
}

static void ppu_mode_hblank(Cpu *cpu, Ppu *ppu)
{
	if (LINE_TICKS - 1 < ppu->line_ticks) {
		increment_ly(cpu, ppu->lcd);
		if (YRES - 1 < ppu->lcd->ly) {
			set_lcd_mode(ppu->lcd, MODE_VBLANK);
			cpu_request_interrupt(cpu, INT_VBLANK);
			if (lcd_stat_is_set(ppu->lcd, STAT_VBLANK)) {
				cpu_request_interrupt(cpu, INT_LCD_STAT);
			}
			ppu->num_frame++;
			update_fps();
		} else {
			set_lcd_mode(ppu->lcd, MODE_OAM);
		}
		ppu->line_ticks = 0;
	}
}

static void ppu_mode_vblank(Cpu *cpu, Ppu *ppu)
{
	if (LINE_TICKS - 1 < ppu->line_ticks) {
		increment_ly(cpu, ppu->lcd);
		if (FRAME_LINES - 1 < ppu->lcd->ly) {
			set_lcd_mode(ppu->lcd, MODE_OAM);
			ppu->lcd->ly = 0;
		}
		ppu->line_ticks = 0;
	}
}

// https://gbdev.io/pandocs/pixel_fifo.html
static void ppu_mode_oam(Ppu *ppu)
{
	if (79 < ppu->line_ticks) {
		set_lcd_mode(ppu->lcd, MODE_TRANSFER);
	}
}

static void ppu_mode_transfer(Ppu *ppu)
{
	if (79 + 172 < ppu->line_ticks) {
		set_lcd_mode(ppu->lcd, MODE_HBLANK);
	}
}

void ppu_tick(Cpu *cpu, Ppu *ppu)
{
	ppu->line_ticks++;

	switch(get_lcd_mode(ppu->lcd)) {
		case MODE_HBLANK:
			ppu_mode_hblank(cpu, ppu);
			break;
		case MODE_VBLANK:
			ppu_mode_vblank(cpu, ppu);
			break;
		case MODE_OAM:
			ppu_mode_oam(ppu);
			break;
		case MODE_TRANSFER:
			ppu_mode_transfer(ppu);
			break;
		default:
			assert(0);
	}
}

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
	assert(offset <= offsetof(Lcd, wx));
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
