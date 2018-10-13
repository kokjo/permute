SOURCES=main.c vec.c util.c disassembler.c dict.c set.c
LIBS=-lcapstone
CFLAGS=-O0 -Wall -g -m32 -fno-pic -fno-pie -no-pie
CC=gcc
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

program: $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf program *.o
