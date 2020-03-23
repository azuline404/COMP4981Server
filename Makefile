CC=g++ -Wall -g
CLIB= -lpthread

all: server

server: tcpServer.o ConnectivityManager.o
	$(CC) tcpServer.o ConnectivityManager.o -o server $(CLIB)

clean:
	rm -f *.o *.bak server 

tcpServer.o:
	$(CC) -c tcpServer.cpp

ConnectivityManager.o: 
	$(CC) -c ConnectivityManager.cpp