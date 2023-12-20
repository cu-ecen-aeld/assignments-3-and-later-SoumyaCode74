ARCH=
CROSS_COMPILE=
CFLAGS=-g -Wall 

ifeq ($(strip $(CROSS_COMPILE)),)
	CROSS_COMPILE=gcc
endif
ifeq ($(strip $(ARCH)),)
	ARCH=x86_64
endif

all: writer

writer: writer.o
	$(CROSS_COMPILE) $(CFLAGS) -I/ writer.o -o writer

writer.o: writer.c
	$(CROSS_COMPILE) $(CFLAGS) -c -o writer.o writer.c

clean:
	rm -rf *.o writer

