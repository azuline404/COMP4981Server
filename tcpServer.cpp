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
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "ConnectivityManager.hpp"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

#define SERVER_TCP_PORT 7000	// Default port
#define BUFLEN	80		//Buffer length
#define TRUE	1

using namespace std;
char * gameObject = "{"
	"\"players\":["
	"{"
                "\"id\":1,"
                "\"x\":0,"
                "\formatname\":\"mp3\","
                "\"title\":\"Zombie\","
                "\"bitrate\":160000,"
                "\"artist\":\"Cranberries\","
                "\"track_number\":\"01\","
                "\"codecname\":\"mp3\""
   "},"
   "{"
                "\"id\":2,"
                "\"x\":0,"
                "\formatname\":\"mp3\","
                "\"title\":\"Zombie\","
                "\"bitrate\":160000,"
                "\"artist\":\"Cranberries\","
                "\"track_number\":\"01\","
                "\"codecname\":\"mp3\""
   "},"
   "{"
                "\"id\":3,"
                "\"x\":0,"
                "\formatname\":\"mp3\","
                "\"title\":\"Zombie\","
                "\"bitrate\":160000,"
                "\"artist\":\"Cranberries\","
                "\"track_number\":\"01\","
                "\"codecname\":\"mp3\""
   "},"
   "{"
                "\"id\":4,"
                "\"x\":0,"
                "\formatname\":\"mp3\","
                "\"title\":\"Zombie\","
                "\"bitrate\":160000,"
                "\"artist\":\"Cranberries\","
                "\"track_number\":\"01\","
                "\"codecname\":\"mp3\""
   "}"
    "]"
"}";

using namespace rapidjson;
void * clientThread(void *arg)
{
	const int test = 1;
    int n;
    int bp;
    int sd = *((int *)arg);
    char buffer[BUFLEN];
	int bytes_to_read = BUFLEN;
    n = recv (sd, buffer, bytes_to_read, 0);
    if (n < 0) {
        printf("didnt recieve anything, recv error");
        exit(1);
    }

    // Document serverDocument;
    // serverDocument.Parse(gameObject.str().c_str());
	// rapidjson::Value::MemberIterator playerIterator = serverDocument.FindMember( "players" );
	// const rapidjson::Value& serverPlayers = serverDocument[ "players" ];
    
	Document playerDocument;
    playerDocument.Parse(buffer);
    
	rapidjson::Value::MemberIterator clientIterator = playerDocument.FindMember( "players" );
	const rapidjson::Value& clientPlayers = playerDocument[ "players" ];

	for (rapidjson::Value::ConstValueIterator itr = clientPlayers.Begin(); itr != clientPlayers.End(); ++itr) {
    const rapidjson::Value& player = *itr;
		for (rapidjson::Value::ConstMemberIterator itr2 = clientPlayers.MemberBegin(); itr2 != clientPlayers.MemberEnd(); ++itr2) {
			std::cout << itr2->id.GetInt() << " : " << itr2->x.GetInt() << std::endl;
		}
	// 	int id = currentPlayer["id"].GetInt();
	// 	int xCoord = currentPlayer["x"].GetInt();
	// 	// Value & playerObject = serverPlayers[id];
	// 	// playerObject["x"].SetInt(xCoord);
	// 	// StringBuffer buffer2;
	// 	// Writer<StringBuffer> writer(buffer2);
	// 	// serverDocument.Accept(writer);
	// 	// std::cout << buffer2.GetString() << std::endl;
	// 	// strcpy(buffer, buffer2.GetString());
	// 	// printf ("sending:%s\n", buffer);
	
	// }
	}
	memset(buffer, 'a', sizeof(buffer));
	send (sd, buffer, BUFLEN, 0);
	// close (sd);
}


int main (int argc, char **argv)
{
    int socketList[30];
    pthread_t tid[30];
	int	n, bytes_to_read;
	int	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;
	char	*bp, buf[BUFLEN];

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
	while (TRUE)
	{
		client_len= sizeof(client);
		if ((new_sd = accept (sd, (struct sockaddr *)&client, (socklen_t*)&client_len)) == -1)
		{
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}
        socketList[i] = new_sd;
        printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));
        if( pthread_create(&tid[i++], NULL, clientThread, &new_sd) != 0 ) {
           printf("Failed to create thread\n");
        }
	}
	close(sd);
	return(0);
}

