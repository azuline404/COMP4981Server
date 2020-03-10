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

#include <sys/sem.h>
#include <semaphore.h>

#define SERVER_TCP_PORT 7000	// Default port
#define BUFLEN	2000		//Buffer length
#define PORT 8080
volatile int UDP_PORT = 12500;

using namespace std;



using namespace rapidjson;

int update_json(char* buffer, const Value *p);

int init_semaphore(int sem_id, key_t key);
void P(int sem_id);
void V(int sem_id);
int remove_semaphore(int sem_id);

int sem_id;

void * clientThread(void *arg)
{
    char writeBuffer[BUFLEN];
    char readBuffer[BUFLEN];
    int sd = *((int *)arg);
    struct	sockaddr_in server;
    int udpSock;
    udpSock = ConnectivityManager::getSocket(ConnectionType::UDP);
    const int i = 1;
    if(setsockopt(udpSock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        perror("SET SOCK OPT FAILED");
    };
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
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, std::to_string(UDP_PORT).c_str());
    printf("\n\n SENDING PORT NUMBER: %s\n", writeBuffer);
    write(sd, writeBuffer, sizeof(writeBuffer));

	const int test = 1;
    int n;
	int bytes_to_read = BUFLEN;
    int count = 1;
    while(true) {
        memset(readBuffer, 0, BUFLEN);
        n = recvfrom (udpSock, readBuffer, sizeof(readBuffer), 0, NULL, NULL);
        //printf("\n\nRECEIVED:\n %s \n\n", readBuffer);
        if (n < 0) {
            printf("didnt recieve anything, recv error");
            exit(1);
        }

        // Document serverDocument;
        // serverDocument.Parse(gameObject);
        // Value & serverPlayers = serverDocument["players"];
        char * tempBuf = readBuffer;
        Document playerDocument;
        playerDocument.Parse(tempBuf);
        Value& players = playerDocument["players"];
    	const Value& currentPlayer = players[0];
        memset(writeBuffer, 0, BUFLEN);
        update_json( writeBuffer,&currentPlayer);
        printf("\n\nclient %d: %d", players[0]["id"].GetInt(), count++);
		//sendto (udpSock, writeBuffer, BUFLEN, 0, (struct sockaddr*)&server, sizeof(server));
    }
	close (sd);
    close(udpSock);
    return NULL;
}

int update_json(char* buffer, const Value *p) {
    P(sem_id);
    FILE* fp = fopen("./gameObject.json", "r");

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
    fp = fopen("./gameObject.json", "w");
    FileWriteStream os(fp, (char*)outputBuffer.GetString(), 65536);
    Writer<FileWriteStream> writer2(os);
    d.Accept(writer2);
    strcpy(buffer, outputBuffer.GetString());

    fclose(fp);
    V(sem_id);
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

    //Init semaphore
    if((sem_id = init_semaphore(sem_id, (key_t)200)) == -1) {
        perror("init_semphore");
    }
    V(sem_id);
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
    remove_semaphore(sem_id);
	return(0);
}

int init_semaphore(int sem_id, key_t key) {
    int status=0;

    if ((sem_id = semget((key_t)key, 1, 0666|IPC_CREAT|IPC_EXCL)) == -1)
    {
        if (errno == EEXIST)
        sem_id = semget ((key_t)key, 1, 0);
    }
    else   /* if created */
        status = semctl (sem_id, 0, SETVAL, 0);
    if ((sem_id == -1) || status == -1)
    {
        perror ("initsem failed\n");
        return (-1);
    }
    else
        return (sem_id);
}

void P(int sem_id) {
 
    struct sembuf *sembuf_ptr;
        
    sembuf_ptr= (struct sembuf *) malloc (sizeof (struct sembuf *) );
    sembuf_ptr->sem_num = 0;
    sembuf_ptr->sem_op = -1;
    sembuf_ptr->sem_flg = SEM_UNDO;

    if ((semop(sem_id,sembuf_ptr,1)) == -1)
	printf("semop error\n");
}

void V(int sem_id) {
struct sembuf *sembuf_ptr;
        
    sembuf_ptr= (struct sembuf *) malloc (sizeof (struct sembuf *) );
    sembuf_ptr->sem_num = 0;
    sembuf_ptr->sem_op = 1;
    sembuf_ptr->sem_flg = SEM_UNDO;

    if ((semop(sem_id,sembuf_ptr,1)) == -1)
	printf("semop error\n");
}

int remove_semaphore(int sem_id) {
    semctl(sem_id, IPC_RMID, 0);
}