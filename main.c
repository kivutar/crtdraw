#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

int screen_width = 320;
int screen_height = 240;

void die(const char *fmt, ...) {
	char buffer[4096];

	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	fputs(buffer, stderr);
	fputc('\n', stderr);
	fflush(stderr);

	exit(EXIT_FAILURE);
}

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

SDL_Cursor* load_png_cursor(const char* filename, int hot_x, int hot_y) {
    SDL_Surface* surf = IMG_Load(filename);
    if (!surf) {
        fprintf(stderr, "Unable to load image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Cursor* cursor = NULL;
    cursor = SDL_CreateColorCursor(surf, hot_x, hot_y);

    if (!cursor)
        fprintf(stderr, "Unable to create cursor: %s\n", SDL_GetError());

    SDL_FreeSurface(surf);

    return cursor;
}

int main(void) {
	if (SDL_VideoInit("KMSDRM") != 0 || SDL_VideoInit(NULL) != 0)
		printf("SDL_VideoInit failed: %s\n", SDL_GetError());

	IMG_Init(IMG_INIT_PNG);

	SDL_Window *const window = SDL_CreateWindow(
		"CRTDRAW",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_width, screen_height, 0);

	if (window == NULL)
		die("SDL_CreateWindow failed: %s\n", SDL_GetError());

	// create a 16x16 surface
	SDL_Surface *const sprite_surf = SDL_CreateRGBSurface(0, 16, 16, 32, 0, 0, 0, 0);
	SDL_FillRect(sprite_surf, NULL, SDL_MapRGB(sprite_surf->format, 255, 255, 255));

	SDL_Surface *win_surf = SDL_GetWindowSurface(window);

	if (win_surf == NULL)
		die("SDL_GetWindowSurface failed: %s\n", SDL_GetError());

    SDL_Cursor* cursor = load_png_cursor("pointer.png", 0, 0);
    SDL_SetCursor(cursor);

	SDL_FillRect(win_surf, NULL, SDL_MapRGB(win_surf->format, 128, 128, 128));
	SDL_UpdateWindowSurface(window);

	int mouse_x = 0;
	int mouse_y = 0;

	int x = 0;
	int y = 0;

	Uint8 r = 0;
	Uint8 g = 0;
	Uint8 b = 0;

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
			x = clamp(x, 0, 16*8 - 1);
			y = clamp(y, 0, 16*8 - 1);

			set_pixel(sprite_surf, x/8, y/8, r, g, b);
		}

		SDL_Rect src_rect = { 0, 0, 16, 16 };
		SDL_Rect dst_rect = { 0, 0, 16*8, 16*8 };
		SDL_BlitScaled(sprite_surf, &src_rect, win_surf, &dst_rect);

		SDL_UpdateWindowSurface(window);
	}

	SDL_FreeCursor(cursor);

	SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}
