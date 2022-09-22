#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include "dma.h"
#include "lcd.h"

// https://gbdev.io/pandocs/OAM.html

#define NUM_SPRITES 40
#define VRAM_SIZE 0x2000
#define OAM_ADDR 0xfe00
#define VRAM_ADDR 0x8000

#define LINE_TICKS 456
#define FRAME_LINES 154
#define XRES 160
#define YRES 144

#define TARGET_FRAME_TIME 1000.0 / 60.0 // 60 FPS

struct oam {
	uint8_t y;
	uint8_t x;
	uint8_t tile;
	uint8_t flags;
};

typedef struct Ppu {
	struct oam oam[NUM_SPRITES];
	uint8_t vram[VRAM_SIZE];
	uint64_t line_ticks;
	uint64_t num_frame;
	Dma *dma;
	Lcd *lcd;
} Ppu;

_Static_assert(sizeof(struct oam) * NUM_SPRITES == 160,
		"size of oam incorrect");


Ppu *ppu_init(void);
void free_ppu(Ppu *ppu);
void ppu_tick(Cpu *cpu, Ppu *ppu);

uint8_t ppu_oam_read(const Ppu *ppu, uint16_t addr);
void ppu_oam_write(Ppu *ppu, uint16_t addr, uint8_t data);
uint8_t ppu_vram_read(const Ppu *ppu, uint16_t addr);
void ppu_vram_write(Ppu *ppu, uint16_t addr, uint8_t data);
void dma_start(Ppu *ppu, uint8_t start);
void lcd_write(Ppu *ppu, uint16_t addr, uint8_t data);

#endif
