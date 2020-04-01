CC=g++ -Wall -lpthread

all: Server Client Client2

Client: defaultTCP.o 
	$(CC) -o Client defaultTCP.o

Client2: defaultTCP2.o
	$(CC) -o Client2 defaultTCP2.o

Server: SelectServer.o ConnectivityManager.o LobbyManager.o Lobby.o Client.o
	$(CC) -o  Server SelectServer.o ConnectivityManager.o LobbyManager.o Lobby.o Client.o

$(CLIB)

clean:
	rm -f *.o core.* Server
	rm -f *.o core.* Client

defaultTCP.o:
	$(CC) -c defaultTCP.c
defaultTCP2.o:
	$(CC) -c defaultTCP2.c
SelectServer.o:
	$(CC) -c SelectServer.cpp
ConnectivityManager.o:
	$(CC) -c ConnectivityManager.cpp		
LobbyManager.o:
	$(CC) -c LobbyManager.cpp
Lobby.o:
	$(CC) -c Lobby.cpp
Client.o:
	$(CC) -c Client.cpp

# CC=g++ -Wall 
# CLIB= -lpthread


# test: main.o Lobby.o LobbyManager.o Client.o   
# 	$(CC) -o test main.o Lobby.o LobbyManager.o Client.o  $(CLIB)


# clean:
# 	rm -f *.o *.bak test

# main.o:
# 	$(CC) -c main.cpp

# Lobby.o:
# 	$(CC) -c Lobby.cpp

# LobbyManager.o:
# 	$(CC) -c LobbyManager.cpp

# client.o: 
# 	$(CC) -c Client.cpp

