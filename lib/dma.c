#include "dma.h"

unsigned dma_transferring(const Dma *dma)
{
	if (dma) {
		return dma->transferring;
	} else {
		return 0;
	}
}
