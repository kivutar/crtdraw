crtdraw:
	$(CC) -o crtdraw main.c -lSDL2 -lSDL2_image

clean:
	rm crtdraw
