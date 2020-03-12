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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>

#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "ConnectivityManager.hpp"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

#define SERVER_TCP_PORT 7000	// Default port
#define GAME_OBJECT_BUFFER 65000
#define BUFLEN	3500		//Buffer length
#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_EVENTS 2
#define PORT_START 12500

using namespace std;
using namespace rapidjson;

struct tStruct {
    int tcpSocket;
    int udpSocket;
    int clientNo;
};

struct circular {
    int writeIndex;
    int readIndex;
    int bufferLength; /*can possibly be replaced with semaphore?*/
    char buffer[MAX_CLIENTS][BUFLEN];
};
/*-----------Function prototypes-----------*/
//JSON interaction functions
int update_json(char* buffer, const Value *p);
//Circular buffer functions
int write_buffer(char* buffer);
int read_buffer();
void set_wIndex();
void set_rIndex();
void print_buffer();
//Semaphore functions
int init_semaphore(int sem_id, key_t key);
void P(int sem_id);
void V(int sem_id);
int remove_semaphore(int sem_id);

int sem_id;
int circular_sem;
volatile int start = 0;
int tCount[MAX_CLIENTS] = {0};
volatile int UDP_PORT = 12500;
struct circular* updates;

int* updateCount;

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
        fflush(stdout);

        char * tempBuf = readBuffer;
        write_buffer(readBuffer);

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
    int pid;
    int tcpSockets[MAX_CLIENTS];

    pthread_t tid[MAX_CLIENTS];
    tStruct* infoArray[MAX_CLIENTS];
    char readBuffer[BUFLEN];
    char writeBuffer[BUFLEN];

    memset(tCount, 0, (MAX_CLIENTS * sizeof(int)));

    updateCount = (int*)malloc(sizeof(int));
    *updateCount = 200;

    //Initialize circular buffer
    updates = (struct circular*)malloc(sizeof(struct circular));
    memset(updates, 0, sizeof(struct circular));
    updates->writeIndex = 0;
    updates->readIndex =0;
    updates->bufferLength = 0;

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

    if((circular_sem = init_semaphore(sem_id, (key_t)12323)) == -1) {
        perror("init circular semaphore");
    }

    if((pid = fork()) == -1) {
        perror("Fork");
    }

    if(pid == 0) {
        while(true) {
            P(circular_sem);
            read_buffer();
        }
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

    for(int x = 0; x < MAX_CLIENTS; x++) {
        printf("Client #: %d\t%d\n", x, tCount[x]);
        close(tcpSockets[x]);
    }
    kill(pid, SIGINT);
    printf("Update Count: %d\n", *updateCount);
	close(sd);
    if(remove_semaphore(sem_id) == -1) {
        perror("Error removing semaphore");
    }
	return(0);
}

int write_buffer(char* buffer) {
    V(circular_sem);
    strcpy(updates->buffer[updates->writeIndex], buffer);
    set_wIndex();
}
//This function should update the JSON?
int read_buffer() {
    printf("Start of read_buffer\n");
    fflush(stdout);
    //Update JSON function here
    printf("Update JSON\n");
    fflush(stdout);
    printf("--%d--\n", *updateCount);
    fflush(stdout);
    memset(updates->buffer[updates->readIndex], 0, BUFLEN);
    set_rIndex();
}
void set_wIndex() {
    if(updates->writeIndex == (MAX_CLIENTS - 1)) {
        updates->writeIndex = 0;
    } else {
        updates->writeIndex++;
    }
}
void set_rIndex() {
    if(updates->readIndex == (MAX_CLIENTS -1)) {
        updates->readIndex = 0;
    } else {
        updates->readIndex++;
    }
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
	printf("semop error P\n");
}

void V(int sem_id) {
struct sembuf *sembuf_ptr;
        
    sembuf_ptr= (struct sembuf *) malloc (sizeof (struct sembuf *) );
    sembuf_ptr->sem_num = 0;
    sembuf_ptr->sem_op = 1;
    sembuf_ptr->sem_flg = SEM_UNDO;

    if ((semop(sem_id,sembuf_ptr,1)) == -1)
	printf("semop error V\n");
}

int remove_semaphore(int sem_id) {
    return(semctl(sem_id, IPC_RMID, 0));
}