#include "ui.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

void ui_init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		printf("error initializing SDL: %s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("GAME",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS);
	if (!window) {
		printf("Unable to create window\n");
		exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, RENDERER_FLAGS);
	if (!renderer) {
		printf("Unable to create renderer\n");
		exit(1);
	}
}

void ui_handle_events(Emulator *emu)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) > 0) {
		if (SDL_WINDOWEVENT == e.type
			&& SDL_WINDOWEVENT_CLOSE == e.window.event) {
			emu_kill(emu);
		}
	}
}
