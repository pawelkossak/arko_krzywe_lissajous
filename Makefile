CC = gcc
ASM = nasm

CFLAGS = -Wall -Wextra $(shell pkg-config --cflags gtk+-3.0)
ASMFLAGS = -f elf64
LDFLAGS = -no-pie $(shell pkg-config --libs gtk+-3.0)

TARGET = interface

all: $(TARGET)

$(TARGET): interface.o lissajous.o
	$(CC) interface.o lissajous.o -o $(TARGET) $(LDFLAGS)

interface.o: interface.c
	$(CC) $(CFLAGS) -c interface.c -o interface.o

lissajous.o: lissajous.asm
	$(ASM) $(ASMFLAGS) lissajous.asm -o lissajous.o

clean:
	rm -f *.o $(TARGET)