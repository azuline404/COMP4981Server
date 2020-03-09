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
#include <pthread.h>
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "ConnectivityManager.hpp"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

#define SERVER_TCP_PORT 7000	// Default port
#define BUFLEN	2000		//Buffer length
#define PORT 8080
volatile int UDP_PORT = 5150;

using namespace std;



using namespace rapidjson;

int update_json(char* buffer, const Value *p);

void * clientThread(void *arg)
{
    char buff[BUFLEN];
    int sd = *((int *)arg);
    struct	sockaddr_in server;
    int udpSock;
    udpSock = ConnectivityManager::getSocket(ConnectionType::UDP);

    // Bind an address to the socket
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(UDP_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	if (bind(udpSock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Can't bind name to socket");
		exit(1);
	}

    //send udp port to client
    memset(buff, 0, sizeof(buff));
    strcpy(buff, std::to_string(UDP_PORT).c_str());
    printf("\n\n SENDING PORT NUMBER: %s\n", buff);
    write(sd, buff, sizeof(buff));

	const int test = 1;
    int n;

    char buffer[BUFLEN];
	int bytes_to_read = BUFLEN;
    n = recvfrom (udpSock, buffer, sizeof(buffer), 0, NULL, NULL);
    printf("\n\nRECEIVED:\n %s \n\n", buffer);
    if (n < 0) {
        printf("didnt recieve anything, recv error");
        exit(1);
    }

    // Document serverDocument;
    // serverDocument.Parse(gameObject);
    // Value & serverPlayers = serverDocument["players"];
    char * tempBuf = buffer;
    Document playerDocument;
    playerDocument.Parse(tempBuf);
    Value& players = playerDocument["players"];
    	const Value& currentPlayer = players[0];
        update_json( buffer,&currentPlayer);
        printf("Sending to client: %s", buffer);
		StringBuffer buffer2;
		Writer<StringBuffer> writer(buffer2);
		// serverDocument.Accept(writer);
		strcpy(buffer, buffer2.GetString());
		printf ("\n\nsending:%s\n", buffer);
		send (sd, buffer, BUFLEN, 0);
	close (sd);
    close(udpSock);
    return NULL;
}

int update_json(char* buffer, const Value *p) {
    FILE* fp = fopen("./gameObject2.json", "r");

    StringBuffer outputBuffer;

    char buff[65536];
    FileReadStream is(fp, buff, sizeof(buff));

    Document d;
    d.ParseStream(is);

    Value & serverPlayers = d["players"];

    int id = (*p)["id"].GetInt();
    int xCoord = (*p)["x"].GetInt();
	Value & playerObject = serverPlayers[id];
	playerObject["x"].SetInt(xCoord);

	Writer<StringBuffer> writer(outputBuffer);
	d.Accept(writer);
    fclose(fp);
    fp = fopen("./gameObject2.json", "w");
    FileWriteStream os(fp, (char*)outputBuffer.GetString(), 65536);
    Writer<FileWriteStream> writer2(os);
    d.Accept(writer2);
    strcpy(buffer, outputBuffer.GetString());

    fclose(fp);
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
	while (true)
	{
		client_len= sizeof(client);
		if ((new_sd = accept (sd, (struct sockaddr *)&client, (socklen_t*)&client_len)) == -1)
		{
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}
        //socketList[i] = new_sd;
        printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));
        if( pthread_create(&tid[i++], NULL, clientThread, &new_sd) != 0 ) {
           printf("Failed to create thread\n");
        }
        UDP_PORT++;
	}
	close(sd);
	return(0);
}

