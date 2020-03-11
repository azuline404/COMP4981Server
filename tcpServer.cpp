#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
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
#define GAME_OBJECT_BUFFER 65000
#define BUFLEN	3500		//Buffer length
#define PORT 8080
#define MAX_CLIENTS 16 
#define MAX_EVENTS 2
#define PORT_START 12500

volatile int UDP_PORT = 12500;

using namespace std;
using namespace rapidjson;

struct tStruct {
    int tcpSocket;
    int udpSocket;
    int clientNo;
};

int update_json(char* buffer, const Value *p);

int init_semaphore(int sem_id, key_t key);
void P(int sem_id);
void V(int sem_id);
int remove_semaphore(int sem_id);
void catch_signal(int sig);

int sem_id;
volatile int start = 0;
int tCount[MAX_CLIENTS] = {0};

void * clientThread(void *t_info)
{
    int *index = (int*) t_info;
    int in = *index;

    printf("in: %d with port: %d\n", in, UDP_PORT);
    int port_number = PORT_START + in;
    int udpSocket;
    char writeBuffer[BUFLEN];
    char gameObjectBuffer[GAME_OBJECT_BUFFER];
    char readBuffer[BUFLEN];
    struct sockaddr_in udpServer;
    memset(&udpServer, 0, sizeof(udpServer));
    udpServer.sin_family = AF_INET;
    udpServer.sin_port = htons(port_number);
    udpServer.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client
    //send udp port to client
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, std::to_string(UDP_PORT).c_str());
    udpSocket = ConnectivityManager::getSocket(ConnectionType::UDP);
    const int i = 1;

    if(setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        perror("SET SOCK OPT FAILED");
    };
    if (bind(udpSocket, (struct sockaddr *)&udpServer, sizeof(udpServer)) == -1)
    {
        perror("Can't bind name to socket");
        exit(1);
    }

    int n;

    printf("starting client %d \n", in);
    while(true) {

        memset(readBuffer, 0, BUFLEN);
        n = recvfrom(udpSocket, readBuffer, sizeof(readBuffer), 0, NULL, NULL);
        if (n < 0) {
            printf("didnt recieve anything, recv error");
            exit(1);
        }
        printf("received from client in %d: \n %s\n", in, readBuffer);

        char * tempBuf = readBuffer;
    
        fflush(stdout);

        Document playerDocument;
        playerDocument.Parse(tempBuf);
        Value& players = playerDocument["players"];
        const Value& currentPlayer = players[0];
            
        memset(gameObjectBuffer, 0, BUFLEN);
        update_json( gameObjectBuffer,&currentPlayer);
        tCount[in]++;
    }
    fflush(stdout);
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
	int	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;
    struct epoll_event tcpEvent, events[MAX_EVENTS];
    int epoll_fd;
    int event_count;
    int n;
    int tcpSockets[MAX_CLIENTS];

    pthread_t tid[MAX_CLIENTS];
    tStruct* infoArray[MAX_CLIENTS];
    char readBuffer[BUFLEN];
    char writeBuffer[BUFLEN];

    memset(tCount, 0, (MAX_CLIENTS * sizeof(int)));
    //initialize infoArray
    for(int i = 0; i < MAX_CLIENTS; i++) {
        infoArray[i] = new tStruct;
    }

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

    if((epoll_fd = epoll_create1(0)) == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor\n");
        exit(1);
    }


	// queue up to 5 connect requests
	listen(sd, 5);
    int i = 0;
    int numOfClients = 0;
	while (true)
	{
		client_len= sizeof(client);
		if ((tcpSockets[numOfClients] = accept (sd, (struct sockaddr *)&client, (socklen_t*)&client_len)) == -1)
		{
			perror("accept client\n");
			exit(1);
        }
        printf("tcp socket created for client %d: %d\n", numOfClients, tcpSockets[numOfClients]);
        tcpEvent.events = EPOLLIN;
        tcpEvent.data.fd = tcpSockets[numOfClients];

        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcpSockets[numOfClients], &tcpEvent))
        {
            fprintf(stderr, "Failed to add tcp file descriptor to epoll\n");
            close(epoll_fd);
            exit(1);
        }
        
        infoArray[numOfClients]->tcpSocket = tcpSockets[numOfClients];
        //infoArray[numOfClients]->tcpSocket = udpSockets[numOfClients];
        infoArray[numOfClients]->clientNo = numOfClients;
        //send udpport to client
        int client_number = numOfClients;
        printf("Client number: %d" , client_number);

        printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));
        if( pthread_create(&tid[numOfClients], NULL, clientThread, &client_number) != 0 ) {
           printf("Failed to create thread\n");
        }
        UDP_PORT++;
        numOfClients++;
        if(numOfClients == MAX_CLIENTS) {
            printf("MAX CLIENT AT: %d\n", numOfClients);
            fflush(stdout);
            //signal all clients to start sending
            for(int i = 0; i < numOfClients; i++) {
                sprintf(writeBuffer,"%d", PORT_START+i);
                printf("sent to client %d udp port %s on socket %d\n", i, writeBuffer, tcpSockets[i]);
                if((send(tcpSockets[i], writeBuffer, sizeof(writeBuffer), 0)) < 0) {
                    perror("send port number failed\n");
                }
            }
            fflush(stdout);
            break;
        }
	}

    int numStopped = 0;
    while(true) {
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);

        for(int i = 0; i < event_count; i++)
        {
            //printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            n = recv(events[i].data.fd, readBuffer, sizeof(readBuffer),0);
            if((strcmp(readBuffer, "stop")) == 0) {
                printf("\n\nclient %d: stop received\n with count %d\n", events[i].data.fd, tCount[1]);
                fflush(stdout);
                
                numStopped++;
                break;
            }
        }
        if(numStopped == MAX_CLIENTS) {
            break;
        }
    }

    for(int i = 0; i < MAX_CLIENTS; i++) {
            printf("%d\t%d\n", i, tCount[i]);
    }

    // for(int z = 0; z < MAX_CLIENTS; z++) {
    //     pthread_join(tid[z], &tCount[z]);
    // }
    for(int x = 0; x < MAX_CLIENTS; x++) {
        printf("Client #: %d\t%d\n", x, tCount[x]);
        close(tcpSockets[x]);
        //close(udpSockets[x]);
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