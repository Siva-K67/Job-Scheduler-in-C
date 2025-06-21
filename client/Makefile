# client/Makefile
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra

all: client

client: client.o
	$(CC) -o client client.o

client.o: client.c include/client.h
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o client

