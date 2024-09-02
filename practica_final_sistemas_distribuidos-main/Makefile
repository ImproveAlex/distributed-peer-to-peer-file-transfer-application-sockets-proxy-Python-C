CC=gcc
CFLAGS=-g -Wall
PROG_CFLAGS=$(CFLAGS)

all: server 

server: server.o implementation.o common.o
	$(CC) $(PROG_CFLAGS) $^ -o $@ -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f server *.o
