CC=g++ -Wall -lpthread

all: Server

Server: main.o ConnectivityManager.o LobbyManager.o Lobby.o Client.o
	$(CC) -o  Server main.o ConnectivityManager.o LobbyManager.o Lobby.o Client.o

$(CLIB)

clean:
	rm -f *.o core.* Server

main.o:
	$(CC) -c main.cpp
ConnectivityManager.o:
	$(CC) -c ConnectivityManager.cpp		
LobbyManager.o:
	$(CC) -c LobbyManager.cpp
Lobby.o:
	$(CC) -c Lobby.cpp
Client.o:
	$(CC) -c Client.cpp

