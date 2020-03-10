#include "Server.hpp"
#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include "ConnectivityManager.hpp"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"


using namespace std;
using namespace rapidjson;
#define BUFLEN 8096
#define SERVER_TCP_PORT 7000


volatile int UDP_PORT = 12500;
LobbyManager * lobbyManager = new LobbyManager();
Client * clientList[20];

string initialResponse(Client *client) {
	const char * json = "{userID:,UDPPort:}";
	Document document;
	document.Parse(json);
	Value & id = document["userID"];
	Value & port = document["UDPPort"];
	id.SetInt(client->getPlayer_Id());
	port.SetInt(client->getUDPPort());
	StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);

	return buffer.GetString();

}


void * clientThread(void *arg)
{
	Client * client = (Client*)arg;
    const int test = 1;
    int n;
    int bp;
    int sd = client->getTCPSocket();
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &test, sizeof(int)) < 0){
	}
    char buffer[BUFLEN];
	int bytes_to_read = BUFLEN;
    n = recv (sd, buffer, bytes_to_read, 0);
    if (n < 0) {
        printf("didnt recieve anything, recv error");
        exit(1);
    }
	string response = initialResponse(client);
	const char * jsonResponse = response.c_str();
	n = send(sd, jsonResponse, sizeof(jsonResponse), 0);
	// Document clientRequest;
	// clientRequest.Parse(buffer);
	// assert(clientRequest.HasMember("messageType"));
	// string request = clientRequest["messageType"].GetString();
	// int action = clientRequest["action"].GetInt();
	// switch(request) {
	// 	case "lobbyRequest":
	// 		switch (action) {
	// 		}
	// 		break;
	// 	case "switchStatusReady":
		
	// 		break;
	// 	case "switchPlayerClass":
	// 		break;
	// }
	close (sd);
    return NULL;
}



int main (int argc, char **argv)
{
    pthread_t tid[30];
	int	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;
	switch(argc)
	{
		case 1:
			port = SERVER_TCP_PORT;	// Use the default port
		break;
		case 2:
			port = atoi(argv[1]);	// Get user specified port
		break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
    }
	// Create a stream socket
	sd = ConnectivityManager::getSocket(ConnectionType::TCP);
    const int j = 0;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int)) < 0) {
        perror("SET TCP SOCK OPT FAILED");
    };
	// Bind an address to the socket
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Can't bind name to socket");
		exit(1);
	}

	// Listen for connections

	// queue up to 5 connect requests
	listen(sd, 20);
    int i = 0;
	int numClients = 0;
	while (true)
	{
		client_len= sizeof(client);
		if ((new_sd = accept (sd, (struct sockaddr *)&client, (socklen_t*)&client_len)) == -1)
		{
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}
		UDP_PORT++;
		Client * newClient = new Client("default", 0, new_sd, UDP_PORT, atoi(inet_ntoa(client.sin_addr)));
		clientList[numClients] = newClient;
        printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));
        if( pthread_create(&tid[i++], NULL, clientThread, newClient) != 0 ) {
           printf("Failed to create thread\n");
        }
	}
	close(sd);
	delete(lobbyManager);
	for (int i = 0; i < numClients; i++) {
		delete(clientList[i]);
	}
	return(0);
}