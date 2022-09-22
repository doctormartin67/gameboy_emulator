void fetcher_push(Fetcher *fetcher, uint32_t data)
{
	struct fifo *f = malloc(sizeof(*f));
	f->next = 0;
	f->data = data;

	if (!fetcher->head) {
		assert(!fetcher->tail);
		fetcher->header = fetcher->tail = f;
	} else {
		fetcher->tail = f;
	}
	fetcher->size++;
}

uint32_t fetcher_pop(Fetcher *fetcher)
{
	assert(fetcher->size > 0);
	assert(fetcher->head);
	uint32_t data = fetcher->head->data;
	free(fetcher->head);
	fetcher->head = fetcher->head->next;
	fetcher->size--;
	return data;
}
