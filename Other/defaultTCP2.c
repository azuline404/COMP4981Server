
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_TCP_PORT		7000	// Default port
#define BUFLEN			1024  	// Buffer length

int main (int argc, char **argv)
{
	int n, bytes_to_read;
	int sd, port;
	struct hostent	*hp;
	struct sockaddr_in server;
	char  *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], **pptr;
	char str[16];

	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	//gets(sbuf); // get user's text
	const char * connect = "\"{\\\"messageType\\\":\\\"connect\\\",\\\"username\\\":\\\"derekleung\\\"}\"";
	const char * create = "\"{\"messageType\":\"lobbyRequest\",\"action\":0,\"userId\":\"1\"}\"";
	const char * getOne = "\"{\"messageType\":\"lobbyRequest\",\"action\":3,\"userId\":\"1\",\"lobbyId\":\"0\"}\"";
	const char * join = "\"{\"messageType\":\"lobbyRequest\",\"action\":4,\"userId\":\"1\",\"lobbyId\":\"0\"}\"";
	const char * getAll = "\"{\"messageType\":\"lobbyRequest\",\"action\":2,\"userId\":\"1\"}\"";
	const char * leave = "\"{\"messageType\":\"lobbyRequest\",\"action\":5,\"userId\":\"1\",\"lobbyId\":\"1\"}\"";
	const char * switchTeam = "\"{\"messageType\":\"switchUserSide\",\"userId\":\"1\",\"lobbyId\":\"0\"}\"";
	const char * switchClass = "\"{\"messageType\":\"switchPlayerClass\",\"classType\":5,\"userId\":\"1\"}\"";
	const char * switchStatus = "\"{\"messageType\":\"switchStatusReady\",\"userId\":\"1\",\"ready\":true}\"";
	const char * startLobby = "\"{\"messageType\":\"startGame\",\"userId\":\"1\",\"lobbyId\":\"0\"}\"";
	const char * playerReady = "\"{\"messageType\":\"playerReady\",\"userId\":\"1\",\"lobbyId\":\"0\"}\"";




	char buf1[1024];
	char buf2[1024];
	char buf3[1024];
	char buf4[1024];
	char buf5[1024];
	char buf6[1024];
	printf("%s\n", "lol");
	/* TEST CONNECTING */
	// Transmit data through the socket
	send (sd, connect, 1000, 0);
	printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;
	n = recv (sd, buf1, 1000, 0);
	printf ("%s\n", buf1);

	send (sd, join, BUFLEN, 0);
	printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;
	n = recv (sd, buf2,1000, 0);
	printf ("%s\n", buf2);


	// send (sd, switchTeam, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// n = recv (sd, buf3, 1000, 0);
	// printf ("%s\n", buf3);

    send (sd, switchStatus, BUFLEN, 0);
	printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;
	n = recv (sd, buf4, 1000, 0);
	printf ("%s\n", buf4);

	n = recv (sd, buf5, 1000, 0);
	printf ("%s\n", buf5);
    sleep(10);

	
	send (sd, playerReady, BUFLEN, 0);
	printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;
    n = recv (sd, buf6, 1000, 0);
	printf ("%s\n", buf6);

	// send (sd, switchStatus, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// // client makes repeated calls to recv until no more data is expected to arrive.
	


	// send (sd, getOne, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// // client makes repeated calls to recv until no more data is expected to arrive.
	// n = recv (sd, buf5, 1000, 0);
	// printf ("%s\n", buf5);

	// send (sd, getAll, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// // client makes repeated calls to recv until no more data is expected to arrive.
	// n = recv (sd, buf6, 1000, 0);
	// printf ("%s\n", buf6);


	// // /* TEST */
	// send (sd, getOne, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// // client makes repeated calls to recv until no more data is expected to arrive.
	// n = recv (sd, rbuf, 1000, 0);
	// printf ("%s\n", rbuf);

	// char newbuf5[1024];
	// send (sd, leave, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// // client makes repeated calls to recv until no more data is expected to arrive.
	// n = recv (sd, newbuf5, 1000, 0);
	// printf ("%s\n", newbuf5);


	// /* TEST GETALL LOBBY */
	// char newbuf[1024];
	// send (sd, newBuf3, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// // client makes repeated calls to recv until no more data is expected to arrive.
	// n = recv (sd, newbuf, 1000, 0);
	// printf ("%s\n", newbuf);

	// /* TEST DESTROY LOBBY*/
	// char tmpbuf[1024];
	// send (sd, destBuf, BUFLEN, 0);
	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// n = recv(sd, tmpbuf, 1000, 0);
	// printf ("%s\n", tmpbuf);
	fflush(stdout);
	close (sd);
	return (0);
}
