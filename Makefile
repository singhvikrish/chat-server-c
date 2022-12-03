CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: client server

client: client.o csapp.o
	$(CC) $(CFLAGS) $(LDLIBS) -o client client.o csapp.o

client.o: client.c
	$(CC) $(CFLAGS) $(LDLIBS) -c client.c

server: server.o csapp.o
	$(CC) $(CFLAGS) $(LDLIBS) -o server server.o csapp.o

server.o: server.c
	$(CC) $(CFLAGS) $(LDLIBS) -c server.c

csapp.o: csapp.c
	$(CC) $(CFLAGS) $(LDLIBS) -c csapp.c

clean:
	rm -rf *~ client server *.o
