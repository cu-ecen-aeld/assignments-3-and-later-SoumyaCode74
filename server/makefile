CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall			
.PHONY: all
all: aesdsocket
writer :
	$(CC) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.o	
	
.PHONY: clean
clean:
	rm -f *.o aesdsocket					
