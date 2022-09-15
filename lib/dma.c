#include "dma.h"

unsigned dma_transferring(const Dma *dma)
{
	return dma->transferring;
}
