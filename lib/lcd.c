#include <stdlib.h>
#include <assert.h>
#include "lcd.h"
#include "common.h"

// https://gbdev.io/pandocs/LCDC.html

Lcd *lcd_init(void)
{
	Lcd *lcd = calloc(1, sizeof(*lcd));
	lcd->lcdc = 0x91;
	lcd->bgp = 0xfc;
	lcd->obp[0] = 0xff;
	lcd->obp[1] = 0xff;

	assert(default_colors[0]);
	assert(sizeof(default_colors)/sizeof(default_colors[0]) == 4);
	for (size_t i = 0; i < 4; i++) {
		lcd->bg_colors[i] = default_colors[i];
		lcd->sprite1_colors[i] = default_colors[i];
		lcd->sprite2_colors[i] = default_colors[i];
	}
	return lcd;
}

uint8_t lcd_read(const Lcd *lcd, uint16_t addr)
{
	assert(addr >= LCD_ADDR);
	uint8_t offset = addr - LCD_ADDR;
	return ((uint8_t *)lcd)[offset];
}

#if 0

static uint16_t get_lcd_control(const Lcd *lcd, LcdControl ctrl)
{
	assert(ctrl < 8);
	unsigned status = BIT(lcd->lcdc, ctrl);
	switch (ctrl) {
		case CTRL_BGW_ENABLE: case CTRL_OBJ_ENABLE:
		case CTRL_WINDOW_ENABLE: case CTRL_LCD_PPU_ENABLE:
			return status;	
		case CTRL_OBJ_SIZE:
			return status ? 16 : 8;
		case CTRL_BG_AREA:
			return status ? 0x9c00 : 0x9800;
		case CTRL_BGW_AREA: case CTRL_W_AREA:
			return status ? 0x8000 : 0x8800;
		default:
			assert(0);
			return 0;
	}
}

// https://gbdev.io/pandocs/STAT.html

static LcdMode get_lcd_mode(const Lcd *lcd)
{
	return lcd->stat & 0x3;
}

static void set_lcd_mode(Lcd *lcd, LcdMode mode)
{
	lcd->stat &= ~0x3;
	lcd->stat |= mode;
}

static unsigned ly_flag_is_set(const Lcd *lcd)
{
	return BIT(lcd->stat, 2);
}

static void set_ly_flag(Lcd *lcd)
{
	if (lcd->ly == lcd->lyc) {
		SET_BIT(lcd->stat, 2, 1);
	}
}

static unsigned lcd_stat_is_set(const Lcd *lcd, StatInt stat)
{
	return BIT(lcd->stat, stat);
}
#endif
