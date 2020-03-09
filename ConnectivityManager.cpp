#include "ConnectivityManager.hpp"

    using namespace rapidjson;
    using namespace std;
int ConnectivityManager::getSocket(ConnectionType type) {
    int sd;
    switch (type) {
        case ConnectionType::TCP:
            sd = socket(AF_INET, SOCK_STREAM, 0);
            break;
        case ConnectionType::UDP:
            sd = socket (AF_INET, SOCK_DGRAM, 0);
            break;
    }
    if (sd == -1) {
        perror ("Can't create a socket"); 
        exit(1);
    }
    const int i = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &i, 0);
    return sd;
}

bool ConnectivityManager::closeSocket(int socket) {
    close(socket);
}


// TO-DO:
// Convert this to send updates using UDP. 
int ConnectivityManager::sendUpdate(int socket, struct sockaddr_in client,char * gameObject){
    char sbuf[1024];
    int client_len = sizeof(client);
    Document d;
    d.Parse(gameObject);
    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    // Output {"project":"rapidjson","stars":11}
    std::cout << "sending: " << buffer.GetString() << std::endl;
    strcpy(sbuf, buffer.GetString());
	// Transmit data through the socket
	sendto(socket, sbuf, sizeof(sbuf), 0,(struct sockaddr*)&client, client_len );
    return 1;
}
// TO-DO:
// close all the sockets once the game is complete.
int ConnectivityManager::cleanup() {

}