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
#include <sys/msg.h>
#include <signal.h>
#include <semaphore.h>

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
#define MAX_CLIENTS 4
#define MAX_EVENTS 2
#define PORT_START 12500

using namespace std;
using namespace rapidjson;

/*
 * TODO
 * 	- Either
 * 		a) add JSON interactions to read_buffer() function
 * 		b) Create a new struct to hold data and only update JSON when preparing to send(Ellaine's idea)
 * 	- Add a second semaphore for the write_buffer() function that interacts with the circular buffer?
 * 	- Tomas: Discuss these fancy things with Ellaine
 * 	- Ellaine: Look through this thing and see if all the stuff I altered today makes sense or
 * 		   if you think we should redo some stuff
 *	- Tomas/Ellaine/Both: Do some clean up
 *		a) Create classes/Connect our stuff into Tommy's classes
 *		b) Seperate into wrapper files (i.e semaphore functions, circular buffer functions, etc)
 *		c) Fix the compiler warnings *low prio as long as things work?*
 */

//Struct for Circular buffer
/*-----The circular buffer operates on a producer-consumer type pattern, do we need 2 semaphores?-----*/
struct circular {
    int writeIndex;
    int readIndex;
    int bufferLength; /*can possibly be replaced with semaphore?*/
    int updateCount; /*temp variable to check if read_buffer() running as intended,
    		       should equal the total # of messages received*/
    char buffer[MAX_CLIENTS][BUFLEN];
};


/*-----------Function prototypes-----------*/
//JSON interaction functions
int update_json(char* buffer, const Value *p);
//Circular buffer functions
int write_buffer(char* buffer);
void * read_buffer(void *t_info);
void set_wIndex();
void set_rIndex();
void print_buffer();


//Semaphore ID's
sem_t countsem, spacesem, writeIndex;

//Global game state object
Document gameState;

int tCount[MAX_CLIENTS] = {0};
volatile int UDP_PORT = 12500;
struct circular* updates;
pthread_mutex_t circularBufferLock;
StringBuffer outputBuffer;
char gameStateBuffer[GAME_OBJECT_BUFFER];

void * clientThread(void *t_info)
{
    int *index = (int*) t_info;
    int in = *index;

    printf("in: %d with port: %d\n", in, UDP_PORT);
    int port_number = PORT_START + in;
    int udpSocket;
    char writeBuffer[BUFLEN];
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
        //printf("received no: %d", tCount[in]++);
        write_buffer(readBuffer);
    }
    fflush(stdout);
    return NULL;
}

void init_gameState() {
    FILE* fp = fopen("./gameObject.json", "r");

    char buff[65536];
    FileReadStream is(fp, buff, sizeof(buff));

    gameState.ParseStream(is);
    fclose(fp);
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

    pthread_t tid[MAX_CLIENTS+1];
    char readBuffer[BUFLEN];
    char writeBuffer[BUFLEN];

    memset(tCount, 0, (MAX_CLIENTS * sizeof(int)));

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

    //Initialize circular buffer
    updates = (struct circular*)malloc(sizeof(struct circular));
    memset(updates, 0, sizeof(struct circular));
    updates->writeIndex = 0;
    updates->readIndex =0;
    updates->bufferLength = 0;
    updates->updateCount = 0;

    if (pthread_mutex_init(&circularBufferLock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 

    //Init semaphores
    
    sem_init(&countsem, 0, 0);
    sem_init(&writeIndex, 0, 1);
    sem_init(&spacesem, 0, 1);
    
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
        
        //send udpport to client
        int client_number = numOfClients;

        if( pthread_create(&tid[numOfClients], NULL, clientThread, &client_number) != 0 ) {
           printf("Failed to create thread\n");
        }
        UDP_PORT++;
        numOfClients++;
        if(numOfClients == MAX_CLIENTS) {
            // printf("MAX CLIENT AT: %d\n", numOfClients);
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
    if( pthread_create(&tid[numOfClients], NULL, read_buffer, NULL) != 0 ) {
           printf("Failed to create thread\n");
    }

    int numStopped = 0;
    while(true) {
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);

        for(int i = 0; i < event_count; i++)
        {
            //printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            n = recv(events[i].data.fd, readBuffer, sizeof(readBuffer),0);
            if((strcmp(readBuffer, "stop")) == 0) {
                fflush(stdout);
                numStopped++;
                break;
            }
        }
        if(numStopped == MAX_CLIENTS) {
            break;
        }
    }

    // printf("\n\n GAME OBJECT: %s", gameStateBuffer);
    // printf("\n UDPATED: %d", updates->updateCount);
    // for(int i = 0; i < MAX_CLIENTS; i++) {
    //         printf("%d\t%d\n", i, tCount[i]);
    // }
    Value &player_stats = gameState["players"];
    for (int x = 0; x < MAX_CLIENTS; x++) {
        printf("Player x value: %d\n", player_stats[x]["x"].GetInt());
    }

    for(int x = 0; x < MAX_CLIENTS; x++) {
        printf("Client #: %d\t%d\n", x, tCount[x]);
        close(tcpSockets[x]);
    }

    close(sd);
    if(sem_destroy(&countsem) == -1) {
        perror("Error removing semaphore");
    }
    if(sem_destroy(&spacesem) == -1) {
        perror("Error removing semaphore");
    }
    if(sem_destroy(&writeIndex) == -1) {
        perror("Error removing semaphore");
    }
    kill(pid, SIGINT);
	return(0);
}

int write_buffer(char* buffer) {
    sem_wait(&spacesem);
    pthread_mutex_lock(&circularBufferLock);
    //only one thread at a time can read and modify write index
   // sem_wait(&writeIndex);
    if (++updates->writeIndex >= MAX_CLIENTS) {
        printf("resetting write index to 0\n");
        updates->writeIndex = 0;
    }
    //sem_post(&writeIndex);
    printf("write index: %d\n",  updates->writeIndex);
    strcpy(updates->buffer[ updates->writeIndex], buffer);
    pthread_mutex_unlock(&circularBufferLock);
    sem_post(&countsem);
}

void * read_buffer(void *t_info) {
    init_gameState();
    Value &player_stats = gameState["players"];
    char readBuffer[BUFLEN];

    while(true) {
        memset(readBuffer, 0, sizeof(readBuffer));

        //read json string from circular buffer
        sem_wait(&countsem);
        pthread_mutex_lock(&circularBufferLock);
        if (++updates->readIndex >= (MAX_CLIENTS)) {
            printf("resetting read index to 0\n");
            updates->readIndex= 0;
        }
        printf("read index: %d\n", updates->readIndex);
        strcpy(readBuffer, updates->buffer[updates->readIndex]);
        Document received;
        received.Parse(readBuffer);
        Value& updatedPlayer = received["players"][0];
        int id = updatedPlayer["id"].GetInt();
        tCount[id]++;
        //printf("outside mutex and sem\n");
        //parse read string to json object 
    
        //printf("received no: %d", tCount[in]++);
        printf("before xCoord\n");
        int xCoord = updatedPlayer["x"].GetInt();
        
        //Value &playerObject = player_stats[id];
	    player_stats[id]["x"] = xCoord;
        printf("player %d: %d\n", id, xCoord);
        //something happens (a count or time) 
        memset(gameStateBuffer, 0, sizeof(gameStateBuffer));
        outputBuffer.Clear();
	    Writer<StringBuffer> writer(outputBuffer);
    
	    gameState.Accept(writer);
        //printf("%s\n\n\n", outputBuffer.GetString());
        strcpy(gameStateBuffer, outputBuffer.GetString());
        updates->updateCount++;
        pthread_mutex_unlock(&circularBufferLock);
        sem_post(&spacesem);
        //outputBuffer now contains updated game object as json string
        //blast out update
    }
}



