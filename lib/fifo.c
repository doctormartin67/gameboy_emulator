#include <stdlib.h>
#include <assert.h>
#include "fifo.h"

FetcherStateMachine *fsm_init(void)
{
	FetcherStateMachine *fsm = calloc(1, sizeof(*fsm));
	fsm->fetcher = calloc(1, sizeof(*fsm->fetcher));
	return fsm;
}

void fetcher_push(Fetcher *fetcher, uint32_t data)
{
	struct fifo *f = malloc(sizeof(*f));
	f->next = 0;
	f->data = data;

	if (!fetcher->head) {
		assert(!fetcher->tail);
		fetcher->head = fetcher->tail = f;
	} else {
		fetcher->tail->next = f;
		fetcher->tail = f;
	}
	fetcher->size++;
}

uint32_t fetcher_pop(Fetcher *fetcher)
{
	assert(fetcher->size > 0);
	assert(fetcher->head);
	uint32_t data = fetcher->head->data;
	struct fifo *next = fetcher->head->next;
	free(fetcher->head);
	fetcher->head = next;
	fetcher->size--;
	return data;
}

void fetcher_reset(Fetcher *fetcher)
{
	while (fetcher->size) {
		(void)fetcher_pop(fetcher);
	}
	fetcher->head = fetcher->tail = 0;
}
