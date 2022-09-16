#include "lcd.h"

// https://gbdev.io/pandocs/LCDC.html

uint16_t get_lcd_control(const Lcd *lcd, LcdControl ctrl)
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

LcdMode get_lcd_mode(const Lcd *lcd)
{
	return lcd->lcds & 0x3;
}

void set_lcd_mode(Lcd *lcd, LcdMode mode)
{
	lcd->lcds &= ~0x3;
	lcd->lcds |= mode;
}

unsigned ly_flag_is_set(const Lcd *lcd)
{
	return BIT(lcd->lcds, 2);
}

void set_ly_flag(Lcd *lcd)
{
	if (lcd->ly == lcd->lyc) {
		SET_BIT(lcd->lcds, 2);
	}
}
