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
#include "ConnectionType.cpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

enum class ConnectionType {TCP,UDP};

class ConnectivityManager {
    public:
        static int getSocket(ConnectionType type);
        static bool closeSocket(int socket);
        static int sendUpdate(int socket,struct sockaddr_in client, char * gameObject);
        static int cleanup();
};