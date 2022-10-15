#include <stdlib.h>
#include <string.h>
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

// https://gbdev.io/pandocs/Scrolling.html
static unsigned window_is_visible(const Lcd *lcd)
{
	return get_lcd_control(lcd, CTRL_WINDOW_ENABLE)
		&& lcd->wx <= 166 && lcd->wy <= YRES;
}

static unsigned drawing_window(const Lcd *lcd)
{
	return window_is_visible(lcd)
		&& lcd->ly >= lcd->wy
		&& lcd->ly < lcd->wy + YRES;
}

// https://gbdev.io/pandocs/pixel_fifo.html
static void increment_ly(Cpu *cpu, Ppu *ppu) {
	if (drawing_window(ppu->lcd)) {
		ppu->wy++;
	}
	ppu->lcd->ly++;
	set_ly_flag(cpu, ppu->lcd);
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

static void ppu_fsm_init(FetcherStateMachine *fsm)
{
	Fetcher *f = fsm->fetcher;
	*fsm = (FetcherStateMachine){0};
	fsm->fetcher = f;
}

static void sprites_reset(Ppu *ppu)
{
	memset(ppu->sprites, 0, sizeof(ppu->sprites));
	ppu->num_sprites = 0;
}

static int sprites_sort(const void *sprite1, const void *sprite2)
{
	const struct oam *tmp1 = sprite1;
	const struct oam *tmp2 = sprite2;

	return tmp1->x - tmp2->x;
}

static void sprites_load(Ppu *ppu)
{
	uint8_t ly = ppu->lcd->ly;	
	uint8_t size = get_lcd_control(ppu->lcd, CTRL_OBJ_SIZE);

	for (size_t i = 0; i < NUM_SPRITES; i++) {
		if (ppu->num_sprites >= MAX_SPRITES_PER_LINE) {
			break;
		}

		struct oam sprite = ppu->oam[i];
		if (sprite.y > ly + 16 || sprite.y + size <= ly + 16) {
			continue; // not on current line
		}
		if (0 == sprite.x) {
			continue; // invisible
		}

		ppu->sprites[ppu->num_sprites++] = sprite;
	}
	qsort(ppu->sprites, ppu->num_sprites, sizeof(struct oam),
			sprites_sort);
}

// https://gbdev.io/pandocs/pixel_fifo.html
static void ppu_mode_oam(Ppu *ppu)
{
	if (OAM_TICKS <= ppu->line_ticks) {
		set_lcd_mode(ppu->lcd, MODE_TRANSFER);
		ppu_fsm_init(ppu->fsm);
	}

	if (1 == ppu->line_ticks) {
		// read OAM on the first tick
		sprites_reset(ppu);
		sprites_load(ppu);
	}
}

static unsigned is_window_tile(const Ppu *ppu)
{
	const Lcd *lcd = ppu->lcd;
	const FetcherStateMachine *fsm = ppu->fsm;
	// TODO: figure out where this 14 comes from??
	return lcd->wx - 7 <= fsm->x_fetched
		&& lcd->wx - 7 + YRES > fsm->x_fetched - 14
		&& lcd->ly >= lcd->wy
		&& lcd->ly < lcd->wy + XRES;
}

static uint8_t get_w_tile_map(const Emulator *emu)
{
	const Ppu *ppu = emu->ppu;
	const Lcd *lcd = ppu->lcd;
	const FetcherStateMachine *fsm = ppu->fsm;
	uint8_t tile_id = ppu->wy / PIXELS * 32
		+ (fsm->x_fetched - (lcd->wx - 7)) / PIXELS;
	uint16_t addr = get_lcd_control(lcd, CTRL_W_MAP_AREA) + tile_id;
	return bus_read(emu, addr);
}

static uint8_t get_bg_tile_map(const Emulator *emu)
{
	const Lcd *lcd = emu->ppu->lcd;
	const FetcherStateMachine *fsm = emu->ppu->fsm;
	uint16_t addr = get_lcd_control(lcd, CTRL_BGW_MAP_AREA) + fsm->tile_id;
	return bus_read(emu, addr);
}

static void fetch_bgw_tile_map(Emulator *emu)
{
	const Lcd *lcd = emu->ppu->lcd;
	FetcherStateMachine *fsm = emu->ppu->fsm;
	if (is_window_tile(emu->ppu) && drawing_window(lcd)) {
		fsm->bgw_tile_map = get_w_tile_map(emu);
	} else {
		fsm->bgw_tile_map = get_bg_tile_map(emu);
	}
	if (0x8800 == get_lcd_control(lcd, CTRL_BGW_DATA_AREA)) {
		// https://gbdev.io/pandocs/Tile_Data.html#vram-tile-data
		fsm->bgw_tile_map += 128;
	}
}

// returns how many pixels are actually visible (first 8 are used to scroll in)
static int visible_x(const Lcd *lcd, uint8_t x)
{
	return (int)(x - (PIXELS - (lcd->scx % PIXELS)));
}

static void fetch_obj_tile_map(Ppu *ppu)
{
	const Lcd *lcd = ppu->lcd;
	FetcherStateMachine *fsm = ppu->fsm;
	int x = 0;
	int dist = 0;
	for (size_t i = 0; i < ppu->num_sprites; i++) {
		x = visible_x(lcd, ppu->sprites[i].x);
		dist = abs(x - fsm->x_fetched);
		if ((dist >= 0) && (dist < PIXELS)) {
			ppu->pixel_sprites[ppu->num_pixel_sprites++]
				= ppu->sprites[i];
		}

		if (ppu->num_pixel_sprites >= MAX_SPRITES_PER_PIXEL) {
			break;
		}
	}
}

static void fetch_tile_map(Emulator *emu)
{
	const Lcd *lcd = emu->ppu->lcd;
	FetcherStateMachine *fsm = emu->ppu->fsm;
	emu->ppu->num_pixel_sprites = 0;
	if (get_lcd_control(lcd, CTRL_BGW_ENABLE)) {
		fetch_bgw_tile_map(emu);
	}

	if (emu->ppu->num_sprites &&
			get_lcd_control(lcd, CTRL_OBJ_ENABLE)) {
		fetch_obj_tile_map(emu->ppu);
	}
	fsm->state = FFS_DATA0;
	fsm->x_fetched += PIXELS;
}

static void fetch_bgw_tile_data(Emulator *emu, FifoFetcherState state)
{
	const Lcd *lcd = emu->ppu->lcd;
	FetcherStateMachine *fsm = emu->ppu->fsm;
	uint16_t addr = get_lcd_control(lcd, CTRL_BGW_DATA_AREA)
		+ fsm->bgw_tile_map * 16 // each tile takes 16 bytes
		+ fsm->line_byte;

	assert(addr < 0xa000);

	if (FFS_DATA0 == state) {
		fsm->bgw_tile_data[0] = bus_read(emu, addr);
		fsm->state = FFS_DATA1;
	} else {
		assert(FFS_DATA1 == state);
		fsm->bgw_tile_data[1] = bus_read(emu, addr + 1);
		fsm->state = FFS_IDLE;
	}
}

static void fetch_obj_tile_data(Emulator *emu, FifoFetcherState state)
{
	const Lcd *lcd = emu->ppu->lcd;
	Ppu *ppu = emu->ppu;
	FetcherStateMachine *fsm = ppu->fsm;
	uint16_t addr = 0;
	uint8_t line_byte = 0;
	uint8_t size = get_lcd_control(lcd, CTRL_OBJ_SIZE);
	uint8_t tile_index = 0;

	for (size_t i = 0; i < ppu->num_pixel_sprites; i++) {
		line_byte = (lcd->ly + 16 - ppu->pixel_sprites[i].y) * 2;
		if (BIT(ppu->pixel_sprites[i].flags, FLAG_Y_FLIP)) {
			line_byte = (size * 2) - 2 - line_byte;
		}
		tile_index = ppu->pixel_sprites[i].tile;
		if (16 == size) {
			tile_index &= ~1; // remove last bit
		}
		addr = VRAM_ADDR + tile_index * 16 + line_byte;
		if (FFS_DATA0 == state) {
			fsm->oam_tile_data[i*2] = bus_read(emu, addr);
		} else {
			assert(FFS_DATA1 == state);
			fsm->oam_tile_data[i*2 + 1] = bus_read(emu, addr + 1);
		}
	}
}

static void fetch_tile_data(Emulator *emu, FifoFetcherState state)
{
	fetch_bgw_tile_data(emu, state);
	fetch_obj_tile_data(emu, state);
}

static uint32_t fetch_sprite_color(const Ppu *ppu, uint32_t color,
		uint8_t bg_color)
{
	const Lcd *lcd = ppu->lcd;
	const FetcherStateMachine *fsm = ppu->fsm;
	int offset = 0; // where color goes in current tile
	uint8_t flags = 0;
	uint8_t hi = 0;
	uint8_t lo = 0;
	uint8_t data0 = 0;
	uint8_t data1 = 0;
	for (size_t i = 0; i < ppu->num_pixel_sprites; i++) {
		flags = ppu->pixel_sprites[i].flags;
		offset = ppu->fsm->x_fifo_pixels;
		offset -= visible_x(lcd, ppu->pixel_sprites[i].x);

		if (BIT(flags, FLAG_BGP) && 0 != bg_color) {
			continue;
		}
		if (offset < 0 || offset >= PIXELS) {
			continue;
		}
		if (BIT(flags, FLAG_X_FLIP)) {
			offset = PIXELS - 1 - offset;
		}
		data0 = fsm->oam_tile_data[i * 2];
		data1 = fsm->oam_tile_data[i * 2 + 1];
		hi = (BIT(data1, PIXELS - 1 - offset) ? 1 : 0) << 1;
		lo = (BIT(data0, PIXELS - 1 - offset) ? 1 : 0);
		if (!(hi | lo)) {
			continue;
		}

		if (BIT(flags, FLAG_PN)) {
			color = lcd->sprite2_colors[hi | lo];
		} else {
			color = lcd->sprite1_colors[hi | lo];
		}
		break; // color found
	}
	return color;
}

static unsigned push_tile_data(Ppu *ppu)
{
	const Lcd *lcd = ppu->lcd;
	FetcherStateMachine *fsm = ppu->fsm;
	if (fsm->fetcher->size > PIXELS) {
		// fifo is full
		return 0;
	}

	if (visible_x(lcd, fsm->x_fetched) < 0) {
		return 1;
	}

	uint8_t hi = 0;
	uint8_t lo = 0;
	uint8_t data0 = fsm->bgw_tile_data[0];
	uint8_t data1 = fsm->bgw_tile_data[1];
	uint32_t color = 0;
	for (unsigned bit = 0; bit < PIXELS; bit++) {
		hi = (BIT(data1, PIXELS - 1 - bit) ? 1 : 0) << 1;
		lo = (BIT(data0, PIXELS - 1 - bit) ? 1 : 0);
		color = lcd->bg_colors[hi | lo];
		if (!get_lcd_control(lcd, CTRL_BGW_ENABLE)) {
			color = lcd->bg_colors[0];
		}

		if (get_lcd_control(lcd, CTRL_OBJ_ENABLE)) {
			color = fetch_sprite_color(ppu, color, hi | lo);
		}
		fetcher_push(fsm->fetcher, color);
		fsm->x_fifo_pixels++;
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
	const Lcd *lcd = ppu->lcd;
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
	if (fsm->x++ < lcd->scx % PIXELS) {
		return;
	}
	size_t addr = fsm->x_pushed + lcd->ly * XRES;
	assert(addr < YRES * XRES);
	ppu->lcd_buf[addr] = data;
	fsm->x_pushed++;
}

static void process_pixel_pipeline(Emulator *emu)
{
	Ppu *ppu = emu->ppu;
	const Lcd *lcd = ppu->lcd;
	FetcherStateMachine *fsm = ppu->fsm;
	uint8_t map_x = (fsm->x_fetched + lcd->scx);
	uint8_t map_y = (lcd->ly + lcd->scy);
	// 32 because the VRAM has 32x32 tile maps
	fsm->tile_id = map_x / PIXELS + map_y / PIXELS * 32;
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

static void ppu_mode_hblank(Cpu *cpu, Ppu *ppu)
{
	if (LINE_TICKS > ppu->line_ticks) {
		return;
	}
	increment_ly(cpu, ppu);
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
	increment_ly(cpu, ppu);
	if (FRAME_LINES > ppu->lcd->ly) {
		ppu->line_ticks = 0;
	} else {
		set_lcd_mode(ppu->lcd, MODE_OAM);
		ppu->lcd->ly = 0;
		ppu->wy = 0;
		ppu->line_ticks = 0;
	}
}

void ppu_tick(Emulator *emu)
{
	Cpu *cpu = emu->cpu;
	Ppu *ppu = emu->ppu;
	ppu->line_ticks++;

	switch(get_lcd_mode(ppu->lcd)) {
		case MODE_OAM:
			ppu_mode_oam(ppu);
			break;
		case MODE_TRANSFER:
			ppu_mode_transfer(emu);
			break;
		case MODE_HBLANK:
			ppu_mode_hblank(cpu, ppu);
			break;
		case MODE_VBLANK:
			ppu_mode_vblank(cpu, ppu);
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
