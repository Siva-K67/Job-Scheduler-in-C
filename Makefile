# Makefile – Cleaned up version

CC       = gcc
CFLAGS   = -Wall -pthread -Iinclude
LDFLAGS  =

CLIENT_OBJS = client/client.o
SERVER_OBJS = server/server.o server/logger.o

all: client.exe server.exe

client.exe: $(CLIENT_OBJS)
	$(CC) $^ $(CFLAGS) -o $@

server.exe: $(SERVER_OBJS)
	$(CC) $^ $(CFLAGS) -o $@ 

client/client.o: client/client.c include/client.h include/jobs.h
	$(CC) -c $< -o $@ $(CFLAGS)

server/server.o: server/server.c include/logger.h include/jobs.h include/technician.h
	$(CC) -c $< -o $@ $(CFLAGS)

server/logger.o: server/logger.c include/logger.h include/jobs.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f client/*.o server/*.o *.exe client.exe server.exe
	rm -rf logs/
