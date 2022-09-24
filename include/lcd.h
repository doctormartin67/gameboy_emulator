#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "cpu.h"

// https://gbdev.io/pandocs/LCDC.html
// --
// https://gbdev.io/pandocs/Palettes.html

#define LCD_ADDR 0xff40
#define LY_ADDR 0xff44
#define BGP_ADDR 0xff47
#define OBP1_ADDR 0xff48
#define OBP2_ADDR 0xff49
#define IS_LCD_ADDR(a) ((a) < 0xff4c && (a) >= LCD_ADDR)

typedef struct Lcd {
	// registers
	uint8_t lcdc;
	uint8_t stat;
	uint8_t scy;
	uint8_t scx;
	uint8_t ly;
	uint8_t lyc;
	uint8_t dma;
	uint8_t bgp;
	uint8_t obp[2];
	uint8_t wy;
	uint8_t wx;

	// other data
	uint32_t bg_colors[4];
	uint32_t sprite1_colors[4];
	uint32_t sprite2_colors[4];
} Lcd;

typedef enum {
	CTRL_BGW_ENABLE,
	CTRL_OBJ_ENABLE,
	CTRL_OBJ_SIZE,
	CTRL_BGW_MAP_AREA,
	CTRL_BGW_DATA_AREA,
	CTRL_WINDOW_ENABLE,
	CTRL_W_MAP_AREA,
	CTRL_LCD_PPU_ENABLE,
} LcdControl;

typedef enum {
	MODE_HBLANK,
	MODE_VBLANK,
	MODE_OAM,
	MODE_TRANSFER,
} LcdMode;

typedef enum {
	STAT_HBLANK = 3,
	STAT_VBLANK,
	STAT_OAM,
	STAT_LY,
} StatInt;

Lcd *lcd_init(void);
uint8_t lcd_read(const Lcd *lcd, uint16_t addr);
uint16_t get_lcd_control(const Lcd *lcd, LcdControl ctrl);
LcdMode get_lcd_mode(const Lcd *lcd);
void set_lcd_mode(Lcd *lcd, LcdMode mode);
void set_ly_flag(Cpu *cpu, Lcd *lcd);
unsigned lcd_stat_is_set(const Lcd *lcd, StatInt stat);

#endif
