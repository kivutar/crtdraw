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

int screen_w = 320;
int screen_h = 240;

int spr_w = 16;
int spr_h = 16;
int scale = 1;
int margin = 16;

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
	const Uint8 b,
	const Uint8 a)
{
	Uint32 *const pixels = (Uint32 *) surface->pixels;

	const Uint32 color = SDL_MapRGBA(surface->format, r, g, b, a);

	pixels[x + (y * surface->w)] = color;
}

void get_pixel(
	SDL_Surface *const surface,
	const int x,
	const int y,
	Uint8 *const r,
	Uint8 *const g,
	Uint8 *const b,
	Uint8 *const a)
{
	Uint32 *const pixels = (Uint32 *) surface->pixels;

	const Uint32 color = pixels[x + (y * surface->w)];

	SDL_GetRGBA(color, surface->format, r, g, b, a);
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

void draw_miniature(SDL_Surface *const src, SDL_Surface *const dst) {
	SDL_Rect src_rect = { 0, 0, spr_w, spr_h };
	SDL_Rect dst_rect = { screen_w - spr_w - margin, screen_h - spr_h - margin, spr_w, spr_h };
	SDL_BlitScaled(src, &src_rect, dst, &dst_rect);
}

void draw_palette(SDL_Surface *const dst) {
	SDL_Rect rect = { margin - 1, margin - 1, 8 + 2, 16*8 + 2 };
	SDL_FillRect(dst, &rect, SDL_MapRGBA(dst->format, 64, 64, 64, 255));
	for (int i = 0; i < 16; i++) {
		SDL_Rect pal_rect = { margin, margin + i * 8, 8, 8 };
		SDL_FillRect(dst, &pal_rect, SDL_MapRGBA(dst->format, i * 16, i * 16, i * 16, 255));
	}
}

int main(int argc, char *argv[]) {
	if (SDL_VideoInit("KMSDRM") != 0 || SDL_VideoInit(NULL) != 0)
		printf("SDL_VideoInit failed: %s\n", SDL_GetError());

	IMG_Init(IMG_INIT_PNG);

	// sprite
	SDL_Surface *spr_srf = NULL;

	// if argc > 1, then argv[1] is the path to the image to load
	if (argc > 1) {
		printf("Loading %s\n", argv[1]);
		spr_srf = IMG_Load(argv[1]);

		if (spr_srf == NULL)
			die("IMG_Load failed: %s\n", IMG_GetError());

		spr_w = spr_srf->w;
		spr_h = spr_srf->h;

	} else {
		spr_srf = SDL_CreateRGBSurface(0, spr_w, spr_h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		SDL_FillRect(spr_srf, NULL, SDL_MapRGBA(spr_srf->format, 255, 255, 255, 255));
	}

	SDL_Window *const window = SDL_CreateWindow(
		"CRTDRAW",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, 0);

	if (window == NULL)
		die("SDL_CreateWindow failed: %s\n", SDL_GetError());


	// overlay
	SDL_Surface *const ovl_srf = SDL_CreateRGBSurface(0, spr_w, spr_h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_FillRect(ovl_srf, NULL, SDL_MapRGBA(ovl_srf->format, 0, 0, 0, 0));

	// window
	SDL_Surface *win_srf = SDL_GetWindowSurface(window);
	SDL_FillRect(win_srf, NULL, SDL_MapRGBA(win_srf->format, 128, 128, 128, 255));

	if (win_srf == NULL)
		die("SDL_GetWindowSurface failed: %s\n", SDL_GetError());

    SDL_Cursor* cursor = load_png_cursor("pointer.png", 0, 0);
    SDL_SetCursor(cursor);

	int x = 0;
	int y = 0;
	scale = (screen_h-64) / spr_h;

	Uint8 r = 0;
	Uint8 g = 0;
	Uint8 b = 0;
	Uint8 a = 255;

	bool quit = false;

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
		}

		Uint32 mouse = SDL_GetMouseState(&x, &y);

		int offset_x = screen_w / 2 - spr_w*scale / 2;
		int offset_y = screen_h / 2 - spr_h*scale / 2;

		x = clamp(x, offset_x, offset_x + spr_w*scale - 1);
		y = clamp(y, offset_y, offset_y + spr_h*scale - 1);

		if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
			set_pixel(spr_srf, (x - offset_x) / scale , (y - offset_y) / scale , r, g, b, a);

		if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT))
			get_pixel(spr_srf, (x - offset_x) / scale , (y - offset_y) / scale , &r, &g, &b, &a);

		// Scaled sprite with border
		SDL_Rect src_rect = { 0, 0, spr_w, spr_h };
		SDL_Rect dst_rect = { offset_x, offset_y, spr_w*scale, spr_h*scale };
		SDL_Rect border_rect = { offset_x - 1, offset_y - 1, spr_w*scale + 2, spr_h*scale + 2 };

		SDL_FillRect(win_srf, &border_rect, SDL_MapRGBA(win_srf->format, 64, 64, 64, 255));

		// Overlay
		SDL_FillRect(ovl_srf, NULL, 0);
		set_pixel(ovl_srf, (x - offset_x) / scale , (y - offset_y) / scale , 255, 0, 0, 128);

		SDL_BlitScaled(spr_srf, &src_rect, win_srf, &dst_rect);
		SDL_BlitScaled(ovl_srf, &src_rect, win_srf, &dst_rect);

		draw_miniature(spr_srf, win_srf);

		draw_palette(win_srf);

		SDL_UpdateWindowSurface(window);
	}

	SDL_FreeCursor(cursor);

	SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}
