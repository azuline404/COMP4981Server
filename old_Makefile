CC=g++ -Wall -g
CLIB= -lpthread

all: server client

server: tcpServer.o ConnectivityManager.o
	$(CC) tcpServer.o ConnectivityManager.o -o server $(CLIB)

client: tcpClient.o ConnectivityManager.o
	$(CC) tcpClient.o ConnectivityManager.o -o client $(CLIB)

clean:
	rm -f *.o *.bak server client

tcpServer.o:
	$(CC) -c tcpServer.cpp

tcpClient.o:
	$(CC) -c tcpClient.cpp

ConnectivityManager.o: 
	$(CC) -c ConnectivityManager.cpp