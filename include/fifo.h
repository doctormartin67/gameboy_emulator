#ifndef FIFO_H
#define FIFO_H

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

 // https://gbdev.io/pandocs/pixel_fifo.html

 typedef enum {
	FFS_TILE,
	FFS_DATA0,
	FFS_DATA1,
	FFS_SLEEP,
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

void fetcher_push(Fetcher *fetcher, uint32_t data);

#endif
