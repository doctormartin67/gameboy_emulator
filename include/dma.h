#ifndef DMA_H
#define DMA_H

#include <stdint.h>

// direct memory access (DMA)
// https://gbdev.io/pandocs/OAM_DMA_Transfer.html

#define DMA_ADDR 0xff46

typedef struct Dma {
	unsigned transferring;
	uint8_t byte;
	uint8_t value;
	uint8_t delay; // 2 cycle delay at start
} Dma;

unsigned dma_transferring(const Dma *dma);

#endif
