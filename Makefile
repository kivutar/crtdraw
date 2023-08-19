TARGET = crtdraw

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS = main.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -lSDL2 -lSDL2_image

clean:
	rm -f $(TARGET) $(OBJS)
