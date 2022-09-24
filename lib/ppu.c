#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include "emulator.h"
#include "ppu.h"
#include "bus.h"
#include "common.h"

Ppu *ppu_init(void)
{
	Ppu *ppu = calloc(1, sizeof(*ppu));
	ppu->lcd_buf = calloc(XRES * YRES, sizeof(*ppu->lcd_buf));
	ppu->lcd = lcd_init();
	ppu->fsm = fsm_init();
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
	if (LINE_TICKS > ppu->line_ticks) {
		return;
	}
	increment_ly(cpu, ppu->lcd);
	if (YRES > ppu->lcd->ly) {
		set_lcd_mode(ppu->lcd, MODE_OAM);
	} else {
		set_lcd_mode(ppu->lcd, MODE_VBLANK);
		cpu_request_interrupt(cpu, INT_VBLANK);
		if (lcd_stat_is_set(ppu->lcd, STAT_VBLANK)) {
			cpu_request_interrupt(cpu, INT_LCD_STAT);
		}
		ppu->num_frame++;
		update_fps();
	}
	ppu->line_ticks = 0;
}

static void ppu_mode_vblank(Cpu *cpu, Ppu *ppu)
{
	if (LINE_TICKS > ppu->line_ticks) {
		return;
	}
	increment_ly(cpu, ppu->lcd);
	if (FRAME_LINES > ppu->lcd->ly) {
		ppu->line_ticks = 0;
	} else {
		set_lcd_mode(ppu->lcd, MODE_OAM);
		ppu->lcd->ly = 0;
		ppu->line_ticks = 0;
	}
}

static void ppu_fsm_init(FetcherStateMachine *fsm)
{
	Fetcher *f = fsm->fetcher;
	*fsm = (FetcherStateMachine){0};
	fsm->fetcher = f;
}

// https://gbdev.io/pandocs/pixel_fifo.html
static void ppu_mode_oam(Ppu *ppu)
{
	if (OAM_TICKS > ppu->line_ticks) {
		return;
	}
	set_lcd_mode(ppu->lcd, MODE_TRANSFER);
	ppu_fsm_init(ppu->fsm);
}

static void fetch_tile_map(Emulator *emu)
{
	Lcd *lcd = emu->ppu->lcd;
	FetcherStateMachine *fsm = emu->ppu->fsm;
	if (!get_lcd_control(lcd, CTRL_BGW_ENABLE)) {
		return;
	}
	uint16_t addr = get_lcd_control(lcd, CTRL_BGW_MAP_AREA) + fsm->tile_id;
	fsm->tile_map = bus_read(emu, addr);
	if (0x8800 == get_lcd_control(lcd, CTRL_BGW_DATA_AREA)) {
		// https://gbdev.io/pandocs/Tile_Data.html#vram-tile-data
		fsm->tile_map += 128;
	}
	fsm->state = FFS_DATA0;
	fsm->x_fetched += PIXELS;
}

static void fetch_tile_data(Emulator *emu, FifoFetcherState state)
{
	Lcd *lcd = emu->ppu->lcd;
	FetcherStateMachine *fsm = emu->ppu->fsm;
	uint16_t addr = get_lcd_control(lcd, CTRL_BGW_DATA_AREA)
		+ fsm->tile_map * 16 // each tile takes 16 bytes
		+ fsm->line_byte;

	if (FFS_DATA0 == state) {
		fsm->tile_data0 = bus_read(emu, addr);
		fsm->state = FFS_DATA1;
	} else {
		assert(FFS_DATA1 == state);
		fsm->tile_data1 = bus_read(emu, addr + 1);
		fsm->state = FFS_IDLE;
	}
}

static unsigned push_tile_data(Ppu *ppu)
{
	Lcd *lcd = ppu->lcd;
	FetcherStateMachine *fsm = ppu->fsm;
	if (fsm->fetcher->size > PIXELS) {
		return 0;
	}

	if ((int)(fsm->x_fetched - (PIXELS - (lcd->scx % PIXELS))) < 0) {
		// tiles on screen actually start at 8 pixels
		return 1;
	}

	uint8_t hi = 0;
	uint8_t lo = 0;
	for (unsigned bit = 0; bit < PIXELS; bit++) {
		hi = (BIT(fsm->tile_data0, PIXELS - 1 - bit) ? 1 : 0) << 1;
		lo = (BIT(fsm->tile_data1, PIXELS - 1 - bit) ? 1 : 0);
		fetcher_push(fsm->fetcher, lcd->bg_colors[hi | lo]);
	}

	return 1;
}

static void fetch_pixel_data(Emulator *emu)
{
	FetcherStateMachine *fsm = emu->ppu->fsm;
	switch (fsm->state) {
		case FFS_TILE:
			fetch_tile_map(emu);
			break;
		case FFS_DATA0:
		case FFS_DATA1:
			fetch_tile_data(emu, fsm->state);
			break;
		case FFS_IDLE:
			fsm->state = FFS_PUSH;
			break;
		case FFS_PUSH:
			if (push_tile_data(emu->ppu)) {
				fsm->state = FFS_TILE;
			}
			break;
		default:
			assert(0);
	}
}

static void push_pixel_data(Ppu *ppu)
{
	Lcd *lcd = ppu->lcd;
	FetcherStateMachine *fsm = ppu->fsm;
	/*
	 * fetcher needs atleast 8 pixels to process next one.
	 * it will remain idle until then.
	 */
	if (fsm->fetcher->size <= PIXELS) {
		return;
	}
	uint32_t data = fetcher_pop(fsm->fetcher);	

	// scroll pixels are just ignored
	if (fsm->x >= lcd->scx % PIXELS) {
		size_t addr = fsm->x_pushed + lcd->ly * XRES;
		ppu->lcd_buf[addr] = data;
		fsm->x_pushed++;
	}
	fsm->x++;
}

static void process_pixel_pipeline(Emulator *emu)
{
	Ppu *ppu = emu->ppu;
	Lcd *lcd = ppu->lcd;
	FetcherStateMachine *fsm = ppu->fsm;
	// 32 because the VRAM has 32x32 tile maps
	uint16_t map_x = (fsm->x_fetched + lcd->scx) / PIXELS;
	uint16_t map_y = (lcd->ly + lcd->scy) / PIXELS * 32;
	fsm->tile_id = map_x + map_y;
	// 2 bytes per line, so this will have a value between 0-15
	fsm->line_byte = ((lcd->ly + lcd->scy) % PIXELS) * 2;

	if (!(ppu->line_ticks & 1)) {
		/*
		 * data is fetched every 2 pushes, so we will fetch when the
		 * line ticks are an even number
		 */
		fetch_pixel_data(emu);
	}
	push_pixel_data(ppu);
}

static void ppu_mode_transfer(Emulator *emu)
{
	Cpu *cpu = emu->cpu;
	Ppu *ppu = emu->ppu;
	process_pixel_pipeline(emu);

	if (XRES > ppu->fsm->x_pushed) {
		return;
	}
	fetcher_reset(ppu->fsm->fetcher);
	set_lcd_mode(ppu->lcd, MODE_HBLANK);
	if (lcd_stat_is_set(ppu->lcd, STAT_HBLANK)) {
		cpu_request_interrupt(cpu, INT_LCD_STAT);
	}
}

void ppu_tick(Emulator *emu)
{
	Cpu *cpu = emu->cpu;
	Ppu *ppu = emu->ppu;
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
			ppu_mode_transfer(emu);
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
