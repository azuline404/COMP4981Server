CC=g++ -Wall -lpthread

all: Server

Server: Server.o messageFunctions.o semaphore.o initsems.o 
	$(CC) -o Server Server.o messageFunctions.o semaphore.o initsems.o

Client: Client.o messageFunctions.o semaphore.o initsems.o 
	$(CC) -o Client Client.o messageFunctions.o semaphore.o initsems.o
$(CLIB)

clean:
	rm -f *.o core.* Server
	rm -f *.o core.* Client
Server.o:
	$(CC) -c Server.c
Client.o:
	$(CC) -c Client.c
messageFunctions.o:
	$(CC) -c messageFunctions.c
semaphore.o:
	$(CC) -c semaphore.c
initsems.o:
	$(CC) -c initsems.c	
