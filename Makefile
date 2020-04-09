CC=g++ -Wall -ggdb
LL=-lpthread

all: Server Client 


Server: SelectServer.o ConnectivityManager.o LobbyManager.o Lobby.o Client.o Main.o
	$(CC) -o  Server SelectServer.o ConnectivityManager.o LobbyManager.o Lobby.o Client.o $(LL)

$(CLIB)

clean:
	rm -f *.o core.* Server
	rm -f *.o core.* Client

Main.o:
	$(CC) -c ./LobbyScene/main.cpp
SelectServer.o:
	$(CC) -c ./GameScene/SelectServer.cpp
ConnectivityManager.o:
	$(CC) -c ./Networking/ConnectivityManager.cpp		
LobbyManager.o:
	$(CC) -c ./LobbyScene/LobbyManager.cpp
Lobby.o:
	$(CC) -c ./LobbyScene/Lobby.cpp
Client.o:
	$(CC) -c ./LobbyScene/Client.cpp

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

