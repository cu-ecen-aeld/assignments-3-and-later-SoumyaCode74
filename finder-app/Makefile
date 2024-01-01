CFLAGS = -g -Wall

ifdef CROSS_COMPILE
	CC = $(CROSS_COMPILE)gcc
else
	CC = gcc
endif

all: writer

writer: writer.o
	$(CC) $(CFLAGS) writer.o -o writer

writer.o: writer.c
	$(CC) $(CFLAGS) -c writer.c -o writer.o

clean:
	rm -f writer
	rm -f *.o
