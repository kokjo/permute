SOURCES=main.c vec.c util.c disassembler.c dict.c set.c reassembler.c bytevec.c elffile.c codec.c
LIBS=-lcapstone
CFLAGS=-Wall -m32 -fno-pic -fno-pie -no-pie -g
CC=gcc
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
FLAG="flag{PLEASE REPLACE THIS FLAG}"

program: $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
#	strip -s program
#	./program $(FLAG)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf program* *.o
