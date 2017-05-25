CC = gcc
CFLAGS = -Wall -g

all: 1 2 3
1: Server.c
	$(CC) $(CFLAGS) Server.c -o Server

2: Client.c
	$(CC) $(CFLAGS) Client.c -o Client

3: Protocol.c
	$(CC) $(FLAGS) Protocol.c -c

clean:
	-rm *.o Server Client


