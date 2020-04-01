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
#define MAX_CLIENTS 1
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
sem_t countsem, spacesem, writeIndex, readIndex, clientsem;

//Global game state object
Document gameState;

int tCount[MAX_CLIENTS] = {0};
volatile int UDP_PORT = 12500;
struct circular* updates;
pthread_mutex_t circularBufferLock;
pthread_mutex_t writeIndexLock;
pthread_mutex_t clientCounterLock;
StringBuffer outputBuffer;
char gameStateBuffer[GAME_OBJECT_BUFFER];
int udpSockets[MAX_CLIENTS];
struct sockaddr_in* clientAddresses[MAX_CLIENTS];
int totalClientsReceived = 0;

void * send_updates(void * info) {
    int count = 0;
    // This semaphore forces the send_update to block until the server receives a UDP signal from all clients
    sem_wait(&clientsem);
    while(true) {
        char currentGameState[GAME_OBJECT_BUFFER];
        strcpy(currentGameState, gameStateBuffer);
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(sendto(udpSockets[i], currentGameState, strlen(currentGameState), 0,(struct sockaddr *)clientAddresses[i], (socklen_t)sizeof(*clientAddresses[i])) < 0) {
                perror("send to failed");
		    }
        }
        printf("sent: %d \n ", count++);
        usleep(50000);
    }
}

void * clientThread(void *t_info)
{
    int *index = (int*) t_info;
    int in = *index;
    printf("in: %d with port: %d\n", in, UDP_PORT);
    int port_number = PORT_START + in;
    char writeBuffer[BUFLEN];
    char readBuffer[BUFLEN];
    struct sockaddr_in udpServer, udpClient;
    memset(&udpServer, 0, sizeof(udpServer));
    memset(&udpClient, 0, sizeof(udpClient));
    udpServer.sin_family = AF_INET;
    udpServer.sin_port = htons(port_number);
    udpServer.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client
    //send udp port to client
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, std::to_string(UDP_PORT).c_str());
    udpSockets[in] = ConnectivityManager::getSocket(ConnectionType::UDP);
    int udpSocket = udpSockets[in];
    const int i = 1;

    if(setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        perror("SET SOCK OPT FAILED");
    };

    if (bind(udpSocket, (struct sockaddr *)&udpServer, sizeof(udpServer)) == -1)
    {
        perror("Can't bind name to socket");
        exit(1);
    }
    int len = sizeof(*clientAddresses[in]);
    int n;
    int sentCount = 0;
    bool first = true;
    printf("starting client %d \n", in);

    // Wait for initial client address
    memset(readBuffer, 0, BUFLEN);
    n = recvfrom(udpSocket, readBuffer, sizeof(readBuffer), 0, (struct sockaddr *)clientAddresses[in], (socklen_t *) &len);
    if (n < 0) {
        perror("didnt recieve anything, recv error");
        exit(1);
    }

    write_buffer(readBuffer);

    // This will release the semaphore to allow the server to begin sending updates to all clients
    pthread_mutex_lock(&writeIndexLock);
    if (++totalClientsReceived == MAX_CLIENTS) {
        sem_post(&clientsem);
    }
    pthread_mutex_unlock(&writeIndexLock);

    while(true) {

        memset(readBuffer, 0, BUFLEN);
        n = recvfrom(udpSocket, readBuffer, sizeof(readBuffer), 0, (struct sockaddr *)clientAddresses[in], (socklen_t *) &len);
        if (n < 0) {
            perror("didnt recieve anything, recv error");
            exit(1);
        }

        //printf("received no: %d", tCount[in]++);
        write_buffer(readBuffer);

        // send client update
        // if(sendto(udpSocket, gameStateBuffer, sizeof(gameStateBuffer), 0,(struct sockaddr *)&udpClient, sizeof(udpClient)) < 0) {
        //     perror("send to\n");
		// }
        // printf("sent %d to client %d\n", ++sentCount, in);
    }
    fflush(stdout);
    return NULL;
}

void init_gameState() {
    FILE* fp = fopen("./gameObject2.json", "r");

    char buff[65536];
    FileReadStream is(fp, buff, sizeof(buff));

    gameState.ParseStream(is);
    fclose(fp);
}

int main (int argc, char **argv)
{
    
	int	sd, client_len, port;
	struct	sockaddr_in server, client;
    struct epoll_event tcpEvent, events[MAX_EVENTS];
    int epoll_fd;
    int event_count;
    int pid;
    int tcpSockets[MAX_CLIENTS];

    pthread_t tid[MAX_CLIENTS+2];
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
    if (pthread_mutex_init(&writeIndexLock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    }
    if (pthread_mutex_init(&clientCounterLock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    //set up client addresses array
     for(int i = 0; i < MAX_CLIENTS; i++) {
         clientAddresses[i] = new struct sockaddr_in;
     }
    //Init semaphores
    
    sem_init(&countsem, 0, 0);
    sem_init(&writeIndex, 0, 1);
    sem_init(&spacesem, 0, MAX_CLIENTS);
    sem_init(&clientsem, 0, 0);
    
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
                sprintf(writeBuffer,"%d,%d", PORT_START+i, i);
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
    numOfClients++;
    if( pthread_create(&tid[numOfClients], NULL, send_updates, NULL) != 0 ) {
           printf("Failed to create thread\n");
    }
    int numStopped = 0;
    while(true) {
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);

        for(int i = 0; i < event_count; i++)
        {
            //printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            recv(events[i].data.fd, readBuffer, sizeof(readBuffer),0);
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
    printf("exiting loop\n");
    for (int x = 0; x < MAX_CLIENTS; x++) {
        printf("Player %d value: %d\n", x, player_stats[x]["x"].GetInt());
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
    if(sem_destroy(&clientsem) == -1) {
        perror("Error removing semaphore");
    }
    kill(pid, SIGINT);
	return(0);
}

int write_buffer(char* buffer) {
    sem_wait(&spacesem);
    pthread_mutex_lock(&circularBufferLock);
    //only one thread at a time can read and modify write index
   sem_wait(&writeIndex);
   pthread_mutex_lock(&writeIndexLock);
    if (++updates->writeIndex >= MAX_CLIENTS) {
        updates->writeIndex = 0;
    }
    pthread_mutex_unlock(&writeIndexLock);
    sem_post(&writeIndex);
    pthread_mutex_unlock(&circularBufferLock);
    strcpy(updates->buffer[ updates->writeIndex], buffer);
    sem_post(&countsem);

    return 1;
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
            updates->readIndex= 0;
        }
        strcpy(readBuffer, updates->buffer[updates->readIndex]);
        Document received;
        received.Parse(readBuffer);
        Value& updatedPlayer = received["updates"][0];
        int id = updatedPlayer["id"].GetInt();
        tCount[id]++;

        player_stats[id] = updatedPlayer;

        memset(gameStateBuffer, 0, sizeof(gameStateBuffer));
        outputBuffer.Clear();
	    Writer<StringBuffer> writer(outputBuffer);
    
	    gameState.Accept(writer);

        strcpy(gameStateBuffer, outputBuffer.GetString());
        updates->updateCount++;
        pthread_mutex_unlock(&circularBufferLock);
        sem_post(&spacesem);

    }
}



