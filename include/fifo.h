#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>

// https://blog.tigris.fr/2019/09/15/writing-an-emulator-the-first-pixel/
/*
   If we don’t count sprites, which are a special case, the Game Boy can handle
   two different tile maps: the background map, which is displayed by default,
   and an optional window that can overlap part of the screen.

   The principle is the same for both: for a given pixel to be displayed in a
   scanline, the fetcher will, in order:

 * Find out in which tile map that pixel is. In the boot ROM case, it’s always
 going to be the background map.
 * Find out on which square in that map the pixel is. This will give it a tile
 ID to look up.
 * Find out on which horizontal line in the associated tile that pixel is.
 * Read the necessary bytes containing graphics data for the 8 pixels in that
 tile line.
 * Store those 8 pixels in a FIFO that the PPU will read from to shift pixels
 out to the display.
 */

// the screen is drawm top to bottom, left to right

// https://gbdev.io/pandocs/pixel_fifo.html

typedef enum {
	FFS_TILE,
	FFS_DATA0,
	FFS_DATA1,
	FFS_IDLE,
	FFS_PUSH,
} FifoFetcherState;

struct fifo {
	struct fifo *next;
	uint32_t data;
};

typedef struct Fetcher {
	struct fifo *head;
	struct fifo *tail;
	long size;
} Fetcher;

// https://gbdev.io/pandocs/Tile_Maps.html

/* 
 * TODO: check if the tile_id is actually needed (could just calculate it
 * directly). same for line_byte
 */
typedef struct FetcherStateMachine {
	FifoFetcherState state;
	Fetcher *fetcher;
	uint8_t x; // current x coordinate (0 <= x < 160)
	uint8_t x_fetched; // amount of pixels fetched from the VRAM on a line
	uint8_t x_pushed; // amount of x pushed on current line (0 <= x < 160)
	uint8_t line_byte; // each tile has 8 lines, (2 bytes per line)
	uint16_t tile_id; // VRAM contains tile maps that use id's to fetch
	uint8_t tile_map;
	uint8_t tile_data0;
	uint8_t tile_data1;
} FetcherStateMachine;

FetcherStateMachine *fsm_init(void);
void fetcher_push(Fetcher *fetcher, uint32_t data);
uint32_t fetcher_pop(Fetcher *fetcher);
void fetcher_reset(Fetcher *fetcher);

#endif
