#include "LobbyManager.hpp"

#define SERVER_TCP_PORT 7000	// Default port
#define BUFLEN	8096	//Buffer length
#define UDP_BUFLEN 56
#define TRUE	1
#define LISTENQ	5
#define MAXLINE 4096
#define  CREATE 0
#define  DESTROY 1
#define  GET_ALL 2
#define GET_ONE 3
#define  JOIN 4
#define  LEAVE 5
#define CIRC_BUFFER_SIZE 20
#define GAME_OBJECT_BUFFER 65000

class Server {
    public:
    private:
};

struct circular {
    int writeIndex;
    int readIndex;
    int bufferLength; /*can possibly be replaced with semaphore?*/
    int updateCount; /*temp variable to check if read_buffer() running as intended,
    		       should equal the total # of messages received*/
    char buffer[CIRC_BUFFER_SIZE][UDP_BUFLEN];
};