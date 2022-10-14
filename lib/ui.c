#include <assert.h>
#include "ui.h"
#include "bus.h"
#include "joypad.h"
#include "common.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static SDL_Surface *surface;

uint32_t default_colors[4];

void ui_init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		printf("error initializing SDL: %s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("GAME",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS);
	assert(window);

	renderer = SDL_CreateRenderer(window, -1, RENDERER_FLAGS);
	assert(renderer);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
	assert(texture);

	surface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32,
			RMASK, GMASK, BMASK, AMASK);
	assert(surface);

	default_colors[0] = SDL_MapRGB(surface->format, COLOR_WHITE,
			COLOR_WHITE, COLOR_WHITE);
	default_colors[1] = SDL_MapRGB(surface->format, COLOR_LIGHT_GREY,
			COLOR_LIGHT_GREY, COLOR_LIGHT_GREY);
	default_colors[2] = SDL_MapRGB(surface->format, COLOR_DARK_GREY,
			COLOR_DARK_GREY, COLOR_DARK_GREY);
	default_colors[3] = SDL_MapRGB(surface->format, COLOR_BLACK,
			COLOR_BLACK, COLOR_BLACK);
}

void delay(uint32_t ms)
{
	SDL_Delay(ms);
}

uint32_t get_ticks(void)
{
	return SDL_GetTicks();
}

/*
 * annoyingly, I could only get this function to work where red and blue are
 * reversed, so I created these macros to enfasize that fact.
 */
#define RED 0x11
#define GREEN 0x11
#define BLUE 0x11

static void fill_surface(void)
{
	uint32_t grey = SDL_MapRGB(surface->format, BLUE, GREEN, RED);
	assert(!SDL_FillRect(surface, 0, grey));
}

#undef RED
#undef GREEN
#undef BLUE

// https://gbdev.io/pandocs/Tile_Data.html
/*
 * 16 * 24 = 384
 */
enum {
	ROWS = 16,
	COLS = 24,
	ROW_SIZE = 2,
	TILE_SIZE = 16,
};

#if 0
static uint16_t read_pixel_row(const Emulator *emu, unsigned row,
		unsigned num_tile)
{
	uint16_t addr 
		= TILE_ADDR
		+ num_tile * TILE_SIZE
		+ row * ROW_SIZE;
	assert(addr > TILE_ADDR - 1);
	assert(addr < TILE_END_ADDR);
	assert(num_tile < ROWS * COLS);

	uint8_t byte1 = bus_read(emu, addr);
	uint8_t byte2 = bus_read(emu, addr + 1);
	return (0x0000 | byte1) | ((0x0000 | byte2) << 8);
}

static void update_pixel(unsigned tile_x, unsigned tile_y, unsigned pixel_row,
		unsigned pixel_col, uint8_t color)
{
	SDL_Rect rc;
	rc.x = (tile_x * PIXELS + pixel_col) * SCALE;
	rc.y = (tile_y * PIXELS + pixel_row) * SCALE;
	rc.w = SCALE;
	rc.h = SCALE;
	uint32_t shade = default_colors[color]; 
	assert(!SDL_FillRect(surface, &rc, shade));
}

static void update_pixel_row(const Emulator *emu, unsigned tile_x,
		unsigned tile_y, unsigned row, unsigned num_tile)
{
	uint16_t data = read_pixel_row(emu, row, num_tile);
	uint8_t byte1 = data;
	uint8_t byte2 = data >> 8;
	uint8_t color = 0;

	for (unsigned col = 0; col < PIXELS; col++) {
		color = (BIT(byte1, PIXELS - 1 - col) ? 1 : 0) << 1;
		color |= (BIT(byte2, PIXELS - 1 - col) ? 1 : 0);
		assert(color < 0x4);
		update_pixel(tile_x, tile_y, row, col, color);
	}
}

static void update_tile(const Emulator *emu, unsigned tile_x, unsigned tile_y,
		unsigned num_tile)
{
	for (unsigned row = 0; row < PIXELS; row++) {
		update_pixel_row(emu, tile_x, tile_y, row, num_tile);
	}
}

static void update_dbg_tiles(const Emulator *emu)
{
	unsigned num_tile = 0;
	for (unsigned y = 0; y < COLS; y++) {
		for (unsigned x = 0; x < ROWS; x++) {
			update_tile(emu, x, y, num_tile++);
		}
	}
}

#endif

static void update_tiles(const Emulator *emu)
{
	SDL_Rect rc = (SDL_Rect){0};
	uint32_t *lcd_buf = emu->ppu->lcd_buf;

	for (unsigned y = 0; y < YRES; y++) {
		for (unsigned x = 0; x < XRES; x++) {
			rc.x = x * SCALE;
			rc.y = y * SCALE;
			rc.w = SCALE;
			rc.h = SCALE;

			SDL_FillRect(surface, &rc, lcd_buf[x + y * XRES]);
		}
	}
}

static void ui_update_window(const Emulator *emu)
{
	fill_surface();
	//update_dbg_tiles(emu);
	update_tiles(emu);

	assert(!SDL_UpdateTexture(texture, 0, surface->pixels,
				surface->pitch));
	assert(!SDL_RenderClear(renderer));
	assert(!SDL_RenderCopy(renderer, texture, 0, 0));
	SDL_RenderPresent(renderer);
}

void ui_update(const Emulator *emu)
{
	ui_update_window(emu);
}

static void ui_kill(void)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

static void ui_handle_key(Emulator *emu, uint32_t key, unsigned is_down)
{
	switch (key) {
		case SDLK_q:
			emu->joypad.a = is_down;
			break;
		case SDLK_w:
			emu->joypad.b = is_down;
			break;
		case SDLK_RETURN:
			emu->joypad.start = is_down;
			break;
		case SDLK_TAB:
			emu->joypad.select = is_down;
			break;
		case SDLK_UP:
			emu->joypad.up = is_down;
			break;
		case SDLK_DOWN:
			emu->joypad.down = is_down;
			break;
		case SDLK_LEFT:
			emu->joypad.left = is_down;
			break;
		case SDLK_RIGHT:
			emu->joypad.right = is_down;
			break;
	}
}

void ui_handle_events(Emulator *emu)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) > 0) {
		if (SDL_KEYDOWN == e.type) {
			ui_handle_key(emu, e.key.keysym.sym, 1);
		}

		if (SDL_KEYUP == e.type) {
			ui_handle_key(emu, e.key.keysym.sym, 0);
		}

		if (SDL_WINDOWEVENT == e.type
				&& SDL_WINDOWEVENT_CLOSE == e.window.event) {
			emu->running = 0;
			ui_kill();
		}
	}
}
