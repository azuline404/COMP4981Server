#include "Server.hpp"
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
#include "ConnectivityManager.hpp"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include <thread>
#include <algorithm>


using namespace std;
using namespace rapidjson;
#define BUFLEN 8096
#define SERVER_TCP_PORT 7000
#define  CREATE 0
#define  DESTROY 1
#define  GET_ALL 2
#define  JOIN 3
#define  LEAVE 4


volatile int UDP_PORT = 12500;
LobbyManager * lobbyManager = new LobbyManager();
Document document;
Client * clientList[20];

int validateJSON(char * buffer) {


	if (document.Parse(buffer).HasParseError()) {
		cout << "Parse error" << endl;
		return 0;
	}
	Value::ConstMemberIterator itr = document.FindMember("messageType");
	if (itr == document.MemberEnd()) {
		cout << "Cannot find message type" << endl;
		return 0;
	}
	return 1;
	
	// string str(buffer);
	// str.erase(std::remove(str.begin(), str.end(), '\\'), str.end());
	// str.erase(0,1);
	// str.pop_back();

	// int n = sizeof(str);
	// char temp[n];
	// strcpy(temp, str.c_str());

	// if (document.Parse(temp).HasParseError()) {
	// 	cout << "Parse error" << endl;
	// 	return 0;
	// }
	// Value::ConstMemberIterator itr = document.FindMember("messageType");
	// if (itr == document.MemberEnd()) {
	// 	cout << "Cannot find message type" << endl;
	// 	return 0;
	// }
	// return 1;
	
}
string initialResponse(Client *client) {
	// const char * json = "{\"userID\":0,\"UDPPort\":0,\"statusCode\":200,\"response\":{\"docs\":[{\"eircode\":\"D02 YN32\"}]}}";
	const char * json = "{\"userID\":0,\"UDPPort\":0,\"statusCode\":200}";
	Document ClientInfo;
	ClientInfo.Parse(json);
	Value & id = ClientInfo["userID"];
	Value & port = ClientInfo["UDPPort"];
	id.SetInt(client->getPlayer_Id());
	port.SetInt(client->getUDPPort());
	StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    ClientInfo.Accept(writer);
	return buffer.GetString();
}


void * clientThread(Client * client)
{
	int sd = client->getTCPSocket();
	const int test = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &test, sizeof(int)) < 0){
	}
	char buffer[BUFLEN];
	int n;
	int bp;
	int bytes_to_read = BUFLEN;
	while (true) {
		cout << "making a recv call" << endl;
		n = recv (sd, buffer, bytes_to_read, 0);
		if (n < 0) {
			printf("didnt recieve anything, recv error\n");
		}
		cout << "Bytes received: " << n << endl;
		const char * clientBuff = buffer;
		printf("Received: %s\n", clientBuff);
		try {
			if (!validateJSON(buffer)) {
				throw std::invalid_argument("bad json object");
			}
			string request = document["messageType"].GetString();

			if (request == "connect") {
				cout << "connect reached" << endl;
				Value::ConstMemberIterator itr = document.FindMember("username");
				if (itr == document.MemberEnd()) {
					throw std::invalid_argument("bad json object");
				}
				string username = document["username"].GetString();
				client->setPlayer_name(username);
				client->setStatus("false");
				string response = initialResponse(client);
				std::cout << "Sending back response: " << response.c_str() << endl;
				int n = send(sd, response.c_str(), response.size(), 0);
				if (n < 0) {
					cout << "Failed to send!" << endl;
				}
			} 
			else {
				cout << "action reached" << endl;
				Value::ConstMemberIterator itr = document.FindMember("action");
				if (itr == document.MemberEnd()) {
					throw std::invalid_argument("bad json object");
				}
				int action = document["action"].GetInt();
				int lobbyID;
				string lobbyResponse;
				if (request == "lobbyRequest") {	
					switch(action) {
						case CREATE:
							cout << "request received to create lobby!" << endl;
							//create lobby, send lobby back
							lobbyID = lobbyManager->createLobby(client);
							lobbyResponse = lobbyManager->getLobby(lobbyID);
							cout << "Lobby response: " << lobbyResponse << endl;
							send(sd, lobbyResponse.c_str(), lobbyResponse.size(), 0);
							break;
						case DESTROY:
							// {
							// 	Value::ConstMemberIterator itr = document.FindMember("lobbyId");
							// 	if (itr == document.MemberEnd()) {
							// 		throw std::invalid_argument("bad json object");
							// 	}
							// 	Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
							// 	lobby->removeAllClients();
							// 	lobbyResponse = lobbyManager->getLobby(lobbyID);
							// 	send(sd, lobbyResponse.c_str(), sizeof(lobbyResponse),0);
							// }
							break;
						case GET_ALL:
							// get a json string containing the lobby list, send the list back
							lobbyResponse = lobbyManager->getLobbyList();
							cout << lobbyResponse << endl;
							send(sd, lobbyResponse.c_str(), lobbyResponse.size(),0);
							break;
						case JOIN:
							{
							// add the client to the provided Lobby ID. return an updated string of the lobby and send it
								Value::ConstMemberIterator itr = document.FindMember("lobbyId");
								if (itr == document.MemberEnd()) {
									throw std::invalid_argument("bad json object");
								}
								lobbyID = document["lobbyId"].GetInt();
								Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
								lobby->addClient(client);
								lobbyResponse = lobbyManager->getLobby(lobbyID);
								cout << "Lobby list response: " << lobbyResponse << endl;
								send(sd, lobbyResponse.c_str(), lobbyResponse.size(),0);
							}
							break;
						case LEAVE:
							// {
							// 	Value::ConstMemberIterator itr = document.FindMember("lobbyId");
							// 	if (itr == document.MemberEnd()) {
							// 		throw std::invalid_argument("bad json object");
							// 	}
							// 	lobbyID = document["lobbyId"].GetInt();
							// 	Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
							// 	lobby->removeClient(client);
							// 	lobbyResponse = lobbyManager->getLobby(lobbyID);
							// 	send(sd, lobbyResponse.c_str(), sizeof(lobbyResponse),0);
							// }
							//lobbymanager.getLobby(id).remove(client);
							// respond with code 200
							break;
					}
				}
				else if (request == "switchUserSide") {
					client->getTeam() == 0? client->setTeam(1) : client->setTeam(0);
					// send response
				}
				else if (request == "switchStatusReady") {
					client->getStatus() == "true"? client->setStatus("false") : client->setStatus("true");
					//send response
				}
				else if (request == "switchPlayerClass") {

				}
			}
		} catch (...) {
			printf("Catching an exception");
			// string errorResponse = "{\"status\":400}";
			// int n = send(sd, errorResponse.c_str(), sizeof(errorResponse), 0);
		}
	}
	close (sd);
    return NULL;
}


int main (int argc, char **argv)
{
    // pthread_t tid[30];
	thread threadList[20];
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
	int numClients = 0;
	while (true)
	{
		client_len= sizeof(client);
		if ((new_sd = accept (sd, (struct sockaddr *)&client, (socklen_t*)&client_len)) == -1)
		{
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}
		UDP_PORT++;
		Client * newClient = new Client("default", 0, new_sd, UDP_PORT, atoi(inet_ntoa(client.sin_addr)));
		clientList[numClients] = newClient;
        printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));
		threadList[i] = std::thread(clientThread, newClient);
		threadList[i].join();
	}
	cout << "exiting while true..?" << endl;
	// close(sd);
	// delete(lobbyManager);
	// for (int i = 0; i < numClients; i++) {
	// 	delete(clientList[i]);
	// }

	return(0);
}