#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

int screen_width = 320;
int screen_height = 240;

int clamp(const int val, const int min, const int max) {
	if (val < min) return min;
	if (val > max) return max;

	return val;
}

void set_pixel(
	SDL_Surface *const surface,
	const int x,
	const int y,
	const Uint8 r,
	const Uint8 g,
	const Uint8 b)
{
	Uint32 *const pixels = (Uint32 *) surface->pixels;

	const Uint32 color = SDL_MapRGB(surface->format, r, g, b);

	pixels[x + (y * surface->w)] = color;
}

int main(void) {
	if (SDL_VideoInit("KMSDRM") != 0) {
		printf("SDL_VideoInit failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window *const window = SDL_CreateWindow(
		"CRTDRAW",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_width, screen_height,
		SDL_WINDOW_FULLSCREEN);

	if (window == NULL) {
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Surface *surface = SDL_GetWindowSurface(window);

	if (surface == NULL) {
		printf("SDL_GetWindowSurface failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 128, 128, 128));
	SDL_UpdateWindowSurface(window);

	int mouse_x = 0;
	int mouse_y = 0;

	int x = 0;
	int y = 0;

	Uint8 r = 255;
	Uint8 g = 255;
	Uint8 b = 255;

	bool quit = false;
	bool drawing = false;

	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = true;
        				break;
    				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				drawing = true;
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				drawing = false;
			}
			else if (event.type == SDL_MOUSEMOTION) {
				mouse_x = event.motion.x;
				mouse_y = event.motion.y;

				x = mouse_x;
				y = mouse_y;
			}
		}

		if (drawing) {
			x = clamp(x, 0, screen_width - 1);
			y = clamp(y, 0, screen_height - 1);

			set_pixel(surface, x, y, r, g, b);
		}
		set_pixel(surface, 50, 50, 255, 0, 0);
		SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
