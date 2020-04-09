#include "Server.hpp"


using namespace std;
using namespace rapidjson;

// Function Prototypes
static void SystemFatal(const char* );


// Globals
volatile int UDP_PORT = 12500;
LobbyManager * lobbyManager = new LobbyManager();
Document document;
std::vector<Client*>clientList;
int write_buffer(char* buffer);
void * read_buffer(void *t_info);
struct circular* circularBuffer;
pthread_mutex_t circularBufferLock, writeIndexLock, clientCounterLock;
sem_t countsem, spacesem, writeIndex, readIndex, clientsem ;
//probably need to remove these
pthread_t readBufferThread;
pthread_t sendUpdateThread;
Document gameState;
char gameStateBuffer[GAME_OBJECT_BUFFER];
StringBuffer outputBuffer;
int tCount[CIRC_BUFFER_SIZE] = {0}; //for testing only
int totalClientsReceived = 0;


void init_gameState() {
    FILE* fp = fopen("./gameObject2.json", "r");

    char buff[65536];
    FileReadStream is(fp, buff, sizeof(buff));

    gameState.ParseStream(is);
    fclose(fp);
}

void initializeSync() {
	sem_init(&countsem, 0, 0);
    sem_init(&writeIndex, 0, 1);
    sem_init(&spacesem, 0, CIRC_BUFFER_SIZE);
	sem_init(&clientsem, 0, 0);
	if (pthread_mutex_init(&circularBufferLock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
    } 
    if (pthread_mutex_init(&writeIndexLock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
    } 
	if (pthread_mutex_init(&clientCounterLock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
    } 
	//initialize circular buffer

    circularBuffer = (struct circular*)malloc(sizeof(struct circular));
    memset(circularBuffer, 0, sizeof(struct circular));
    circularBuffer->writeIndex = 0;
    circularBuffer->readIndex =0;
    circularBuffer->bufferLength = 0;
    circularBuffer->updateCount = 0;
}

int validateJSON(char * buffer, bool lobby) {
	/* This portion is used to test with your own client */
	// if (document.Parse(buffer).HasParseError()) {
	// 	cout << "Parse error" << endl;
	// 	return 0;
	// }
	// Value::ConstMemberIterator itr = document.FindMember("messageType");
	// if (itr == document.MemberEnd()) {
	// 	cout << "Cannot find message type" << endl;
	// 	return 0;
	// }
	// return 1;
	/*													*/

	// THIS PART IS REQUIRED IN ORDER TO PARSE THE CLIENT API'S REQUEST DUE TO ADDED BACKSLASHES
	/*				
																			*/
	printf("%s\n", buffer);
	string str(buffer);
	str.erase(std::remove(str.begin(), str.end(), '\\'), str.end());
	str.erase(0,1);
	str.pop_back();

	int n = str.length();
	char temp[n];
	strcpy(temp, str.c_str());

	if (document.Parse(temp).HasParseError()) {
		cout << "Parse error" << endl;
		return 0;
	}
	if(lobby) {
		Value::ConstMemberIterator itr = document.FindMember("messageType");
		if (itr == document.MemberEnd()) {
			cout << "Cannot find message type" << endl;
			return 0;
		}
	}
	return 1;
	/*																						*/
	
}

void broadcastStartGame(Lobby * lobby) {
	string startLobbyResponse = "{\"statusCode\":200,\"responseType\":\"start\",\"lobbyID\":";
	startLobbyResponse = startLobbyResponse + to_string(lobby->getId()) + "," + "\"startGame\"" + ":\"true\"}";

	vector<Client*> clientList = lobby->getClientList();
	for (auto it = clientList.begin(); it != clientList.end(); it++) {
		cout << "sent start game!" << endl;
		send((*it)->getTCPSocket(), startLobbyResponse.c_str(),startLobbyResponse.size(), 0);
	}
}
void broadcastStartLobby(Lobby * lobby) {
	cout << "Broadcasting start lobby!" << endl;
	const char * gameObject = "{\"statusCode\":200,\"responseType\":\"load\",\"Player\":{\"playerName\":\"tempPlayer\",\"id\":-1,\"classType\":-1,\"ready\":\"false\",\"team\":-1},\"players\":[],\"crystals\":[],\"attackObject\":[],\"gameState\":1}";
	Document ClientInfo;
	ClientInfo.Parse(gameObject);
	vector<Client*> clientList = lobby->getClientList();
	for (auto it = clientList.begin(); it != clientList.end(); it++) {
		Value & clientUsername = ClientInfo["Player"]["playerName"];
		Value & clientID = ClientInfo["Player"]["id"];
		Value & clientClass = ClientInfo["Player"]["classType"];
		Value & clientReady = ClientInfo["Player"]["ready"];
		Value & clientTeam = ClientInfo["Player"]["team"];
		clientUsername.SetString((*it)->getPlayer_name().c_str(),ClientInfo.GetAllocator());
		clientID.SetInt((*it)->getPlayer_Id());
		clientClass.SetInt((*it)->getCharacterClass());
		clientReady.SetString((*it)->getStatus().c_str(), ClientInfo.GetAllocator());
		clientTeam.SetInt((*it)->getTeam());
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		ClientInfo.Accept(writer);
		string response = buffer.GetString();
		send((*it)->getTCPSocket(), response.c_str(), response.size(), 0);
		cout << "broadcasted start lobby once" << endl;
	}
}
/*["Player"]["playerName"]
	This function is used to send the initial respones back to the client when they connect to the server
*/
string connectResponse(Client *client) {
	const char * json = "{\"userId\":0,\"UDPPort\":0,\"statusCode\":200,\"response\":{\"docs\":[{\"eircode\":\"D02 YN32\"}]}}";
	// const char * json = "{\"statusCode\":200,\"userID\":0,\"UDPPort\":0}";
	Document ClientInfo;
	ClientInfo.Parse(json);
	Value & id = ClientInfo["userId"];
	Value & port = ClientInfo["UDPPort"];
	id.SetInt(client->getPlayer_Id());
	port.SetInt(client->getUDPPort());
	StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    ClientInfo.Accept(writer);
	return buffer.GetString();
}

/*
	Temporary wrapper function that prints out the response to be sent, and sends it to the the specified socket
*/
int sendResponse(int socket, string response) {
    std::cout << "Sending back response: " << response.c_str() << endl;
	int n = send(socket, response.c_str(), response.size(), 0);
    return n;
}

/*
	Temporary function that will retrieve a client object from the client list based on the Id.
	Might create a ClientManager class that will handle this function.
*/
Client * getClient(int playerId) {
    for (int j = 0; j < clientList.size(); j++) {
        if (clientList[j]->getPlayer_Id() == playerId) {
            return clientList[j];
            }
    }
    return NULL;
}

void broadcastLobbyUpdate(Lobby * lobby) {
	vector<Client*>clientsInLobby = lobby->getClientList();
	string response = lobbyManager->getLobby(lobby->getId());
	int sent = 0;
	for (auto it = clientsInLobby.begin(); it != clientsInLobby.end(); it++) {
		sent = sendResponse((*it)->getTCPSocket(), response);
		if (sent == 0) {
			cout << "Failed to send!";
		}
	}
}

void * send_updates(void * clientptr) {
    int count = 0;
	sem_wait(&clientsem);
	
    while(count < 2001) {
        char currentGameState[GAME_OBJECT_BUFFER];
        strcpy(currentGameState, gameStateBuffer);
        for(int i = 0; i < clientList.size(); i++) {
			int len = sizeof( *(clientList[i]->getUdpAddress()));
			struct sockaddr * cli = (struct sockaddr *) clientList[i]->getUdpAddress();
            if(sendto(clientList[i]->getUDPSocket(), currentGameState, sizeof(currentGameState), 0,(struct sockaddr *)clientList[i]->getUdpAddress(), (socklen_t) len) < 0) {
                perror("send to\n");
		    } else {
        		printf("sent: %d \n ", count++);
			}
        }
        usleep(33000);
    }
}

void * read_buffer(void *t_info) {
    // Value &player_stats = gameState["players"];
    char readBuffer[UDP_BUFLEN];

    while(true) {
        memset(readBuffer, 0, sizeof(readBuffer));

        //read json string from circular buffer
        sem_wait(&countsem);
        pthread_mutex_lock(&circularBufferLock);

        if (++circularBuffer->readIndex >= CIRC_BUFFER_SIZE) {
            circularBuffer->readIndex= 0;
        }
        strcpy(readBuffer, circularBuffer->buffer[circularBuffer->readIndex]);
		validateJSON(readBuffer, false);
		//gameState[0] = document[0];
        // int id = updatedPlayer["id"].GetInt();
        // tCount[id]++;

        // player_stats[id] = updatedPlayer;

        // memset(gameStateBuffer, 0, sizeof(gameStateBuffer));

        outputBuffer.Clear();
	    Writer<StringBuffer> writer(outputBuffer);
    
	    // gameState.Accept(writer);
		
		document.Accept(writer);
		//printf("READ_BUFFER: %s\n", outputBuffer.GetString());
		memset(gameStateBuffer, 0, strlen(gameStateBuffer));
		strcpy(gameStateBuffer, outputBuffer.GetString());
		gameStateBuffer[outputBuffer.GetSize()] = '\n';
        circularBuffer->updateCount++;
        pthread_mutex_unlock(&circularBufferLock);
        sem_post(&spacesem);
    }
}

//CALLED BY CLIENT THREADS TO WRITE UPDATES TO CIRCULAR BUFFER
int write_buffer(char* buffer) {
    sem_wait(&spacesem);
    pthread_mutex_lock(&circularBufferLock);
    //only one thread at a time can read and modify write index
   sem_wait(&writeIndex);
   pthread_mutex_lock(&writeIndexLock);
    if (++circularBuffer->writeIndex >= CIRC_BUFFER_SIZE) {
        circularBuffer->writeIndex = 0;
    }
    pthread_mutex_unlock(&writeIndexLock);
    sem_post(&writeIndex);
    pthread_mutex_unlock(&circularBufferLock);
    strcpy(circularBuffer->buffer[ circularBuffer->writeIndex], buffer);
    sem_post(&countsem);

    return 1;
}

void * clientThread(void *info)
{
    Client* client = (Client*) info;

    char writeBuffer[BUFLEN];
    char readBuffer[UDP_BUFLEN];
    struct sockaddr_in udpServer, udpClient;
    memset(&udpServer, 0, sizeof(udpServer));
    memset(&udpClient, 0, sizeof(udpClient));
    udpServer.sin_family = AF_INET;
    udpServer.sin_port = htons(client->getUDPPort());
    udpServer.sin_addr.s_addr = htonl(INADDR_ANY); 
    //send udp port to client
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, std::to_string(UDP_PORT).c_str());
    int udpSocket;
	if ( (udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    const int i = 1;
    if(setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        perror("SET SOCK OPT FAILED");
    };

    if (bind(udpSocket, (struct sockaddr *)&udpServer, sizeof(udpServer)) == -1)
    {
        exit(1);
    }
	client->setUDPSocket(udpSocket);
    int n;
    int testCount = 0; //remove later
    bool first = true;
	struct sockaddr_in cliaddr; 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	int len = sizeof(cliaddr);
	
    while(true) {
        if (first) {
			n = recvfrom(udpSocket, readBuffer, sizeof(readBuffer), 0, (struct sockaddr *)&cliaddr, (socklen_t *) &len);
				if (n < 0) {
					perror("didnt recieve anything, recv error");
					exit(1);
				} else {
					client->setUDPAddress(&cliaddr);
					struct sockaddr_in* test = client->getUdpAddress();
					printf("client address saved!\n");
					sem_post(&clientsem);
				}
			first = false;
		} else {
			n = recvfrom(udpSocket, readBuffer, sizeof(readBuffer), 0, NULL, NULL);
				if (n < 0) {
					perror("didnt recieve anything, recv error");
					exit(1);
				}
		}
    
        printf("received from client %d: %d\n", client->getPlayer_Id(), ++testCount);
        write_buffer(readBuffer);
    }
    fflush(stdout);
    close(udpSocket);
    return NULL;
}

void createClientAndUpdateThreads(Lobby *lobby) {
	vector<Client*>clientsInLobby = lobby->getClientList();
	vector<pthread_t> threads = lobby->getThreadIds();
	for(int i = 0; i < lobby->getClientList().size(); i++){
		pthread_t tid;
		if( pthread_create(&tid, NULL, clientThread, clientsInLobby[i]) != 0 ) {
			printf("Failed to create thread\n");
		} else {
			threads.push_back(tid);
		}
	}

	if( pthread_create(&readBufferThread, NULL, read_buffer, NULL) != 0 ) {
           printf("Failed to create thread\n");
    }
	
	if( pthread_create(&sendUpdateThread, NULL, send_updates, NULL) != 0 ) {
           printf("Failed to create thread\n");
    }

}

/*
	Start of the program.
*/
int main (int argc, char **argv)
{
    char buffer[BUFLEN];
	int i, maxi, nready, bytes_to_read, arg;
	int listen_sd, new_sd, sockfd, client_len, port, maxfd, client[FD_SETSIZE];
	struct sockaddr_in server, client_addr;
	char *bp, buf[BUFLEN];
   	ssize_t n;
   	fd_set rset, allset;

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
	if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		SystemFatal("Cannot Create Socket!");
	
	// set SO_REUSEADDR so port can be resused imemediately after exit, i.e., after CTRL-c
        arg = 1;
        if (setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
                SystemFatal("setsockopt");

	// Bind an address to the socket
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	if (bind(listen_sd, (struct sockaddr *)&server, sizeof(server)) == -1)
		SystemFatal("bind error");
	
	// Listen for connections
	listen(listen_sd, 20);

	maxfd = listen_sd;	// initialize
   	maxi  = -1;		// index into client[] array

	for (i = 0; i < FD_SETSIZE; i++) {
           	client[i] = -1;  // -1 indicates available entry      
    } 
 	FD_ZERO(&allset);
   	FD_SET(listen_sd, &allset);
	int sent;
	while (TRUE)
	{
   		rset = allset;               // structure assignment
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_sd, &rset)) {// new client connection
            client_len = sizeof(client_addr);
            if ((new_sd = accept(listen_sd, (struct sockaddr *) &client_addr, (socklen_t*)&client_len)) == -1)
				SystemFatal("accept error");
                printf(" Remote Address:  %s\n", inet_ntoa(client_addr.sin_addr));
            for (i = 0; i < FD_SETSIZE; i++)
			if (client[i] < 0) {
				client[i] = new_sd;	// save descriptor
				break;
            }
			if (i == FD_SETSIZE) {
				printf ("Too many clients\n");
            	exit(1);
    		}
			FD_SET (new_sd, &allset);     // add new descriptor to set
			if (new_sd > maxfd) {
				maxfd = new_sd;	// for select
            }

			if (i > maxi) {
				maxi = i;	// new max index in client[] array
            }

			if (--nready <= 0)
				continue;	// no more readable descriptors
     	}

		for (i = 0; i <= maxi; i++)	{// check all clients for data
			if ((sockfd = client[i]) < 0)
				continue;

        if (FD_ISSET(sockfd, &rset)) {
            bp = buf;
            bytes_to_read = BUFLEN;
            n = recv (sockfd, buffer, bytes_to_read, 0);
    		if (n == 0) { // connection closed by
            printf(" Remote Address:  %s closed connection\n", inet_ntoa(client_addr.sin_addr));
            close(sockfd);
            FD_CLR(sockfd, &allset);
            client[i] = -1;
        	}
			else {
			try {

            // Validate the JSON object received
			if (!validateJSON(buffer, true)) {
				throw std::invalid_argument("bad json object");
			}
            // Check the message type, can either be connect or a lobby request
			string request = document["messageType"].GetString();
			cout << "Client Request: " << request << endl;
			if (request == "connect") {
				cout << "A new client has connected to the server!" << endl;
                //  Create a new client, add it to the list, and return its id and UDP Port number.
                Client * newClient = new Client(0, 0, new_sd, UDP_PORT++, atoi(inet_ntoa(client_addr.sin_addr)));
		        clientList.push_back(newClient);
				Value::ConstMemberIterator itr = document.FindMember("username");
				if (itr == document.MemberEnd()) {
					throw std::invalid_argument("bad json object");
				}
				string username = document["username"].GetString();
				newClient->setPlayer_name(username);
				newClient->setStatus("false");
				string response = connectResponse(newClient);
                if ((sent = sendResponse(sockfd, response)) < 0)
					cout << "Failed to send!" << endl;
			} 
			else {
                // Ensure that the id is part of the request.
                Value::ConstMemberIterator itr = document.FindMember("userId");
				if (itr == document.MemberEnd()) {
					throw std::invalid_argument("bad json object");
				}
				string playerString = document["userId"].GetString();
				int playerId = std::stoi(playerString);
                // int playerId = document["userId"].GetInt();
                Client * clientObj = getClient(playerId);
                if (clientObj == NULL) {
                    cout << "Couldn't retrieve client based on Id" << endl;
                    continue;
                }
				// Value::ConstMemberIterator itr = document.FindMember("lobbyId");
				// 	if (itr == document.MemberEnd()) {
				// 	throw std::invalid_argument("bad json object");
				// }
				int lobbyID;
				string lobbyResponse;
				if (request == "lobbyRequest") {	
					itr = document.FindMember("action");
					if (itr == document.MemberEnd()) {
						throw std::invalid_argument("bad json object");
					}
					int action = document["action"].GetInt();
					switch(action) {
						case CREATE:
							{
							cout << "Received client request to create lobby!" << endl;
							//create lobby, send lobby back
							lobbyID = lobbyManager->createLobby(clientObj);
							// lobbyResponse = lobbyManager->getLobby(lobbyID);
							// if ((sent= sendResponse(sockfd, lobbyResponse)) < 0)
					        //     cout << "Failed to send!" << endl;
							Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
							broadcastLobbyUpdate(lobby);
							}
							break;
						case DESTROY:
							{
								cout << "Request received to destroy the lobby!" << endl;
								lobbyID = lobbyManager->verifyLobbyOwner(clientObj);
								if (lobbyID == -1)
								{
									cout << "The current client is not a Lobby owner!" << endl;
									break;
								}
								Lobby* lobby1 = lobbyManager->getLobbyObject(lobbyID);
								lobby1->removeAllClients();
								lobbyManager->deleteLobby(lobbyID);
								lobbyResponse = lobbyManager->getLobbyList();
								if ((sent = sendResponse(sockfd, lobbyResponse)) < 0)
					            cout << "Failed to send!" << endl;
							}
							break;
						case GET_ONE:
							cout << "Received client request to get one!" << endl;
							lobbyID = std::stoi(document["lobbyId"].GetString());
							lobbyResponse = lobbyManager->getLobby(lobbyID);
								if ((sent = sendResponse(sockfd, lobbyResponse)) < 0)
					            cout << "Failed to send!" << endl;
							break;	
						case GET_ALL:
							cout << "Received client request for lobby list!" << endl;
							lobbyResponse = lobbyManager->getLobbyList();
                            if ((sent = sendResponse(sockfd, lobbyResponse)) < 0)
					            cout << "Failed to send!" << endl;
							break;
						case JOIN:
							{
								Value::ConstMemberIterator itr = document.FindMember("lobbyId");
								if (itr == document.MemberEnd()) {
									throw std::invalid_argument("bad json object");
								}
								lobbyID = std::stoi(document["lobbyId"].GetString());
								Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
								if (lobby->getStatus() != "inactive") {
									throw std::invalid_argument("lobby is currently active");
								}
								lobby->addClient(clientObj);
								lobbyResponse = lobbyManager->getLobby(lobbyID);
								string joinResponse = "{\"statusCode\":200}";
								if ((sent = sendResponse(sockfd, lobbyResponse)) < 0) {
					            	cout << "Failed to send!" << endl;
								}
								broadcastLobbyUpdate(lobby);
							}
							break;
						case LEAVE:
							{
								Value::ConstMemberIterator itr = document.FindMember("lobbyId");
								if (itr == document.MemberEnd()) {
									throw std::invalid_argument("bad json object");
								}
								lobbyID = std::stoi(document["lobbyId"].GetString());
								Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
								lobby->removeClient(clientObj);
								if (lobby->getCurrentPlayers() == 0) {
									lobbyManager->deleteLobby(lobbyID);
								}
								else {
									broadcastLobbyUpdate(lobby);
								}
                                // lobbyResponse = lobbyManager->getLobby(lobbyID);
								// if ((sent = sendResponse(sockfd, lobbyResponse)) < 0)
					            // cout << "Failed to send!" << endl;
							}
							break;
					}
				}
				else if (request == "switchUserSide") {
					cout << "Received client request to switch teams!" << endl;
					clientObj->getTeam() == 0? clientObj->setTeam(1) : clientObj->setTeam(0);
					lobbyID = std::stoi(document["lobbyId"].GetString());
					lobbyResponse = lobbyManager->getLobby(lobbyID);
					Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
					broadcastLobbyUpdate(lobby);
				}
				else if (request == "switchStatusReady") {
					cout << "Received client request to switch status!" << endl;
					clientObj->getStatus() == "true"? clientObj->setStatus("false") : clientObj->setStatus("true");
					lobbyID = clientObj->getLobby_Id();
					lobbyResponse = lobbyManager->getLobby(lobbyID);
					Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
					broadcastLobbyUpdate(lobby);
				}
				else if (request == "switchPlayerClass") {
					itr = document.FindMember("classType");
					if (itr == document.MemberEnd()) {
						throw std::invalid_argument("bad json object");
					}
					int newClass = document["classType"].GetInt();
					clientObj->setCharacterClass(newClass);
					lobbyID = clientObj->getLobby_Id();
					lobbyResponse = lobbyManager->getLobby(lobbyID);
					Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
					broadcastLobbyUpdate(lobby);
				}
				else if (request == "startGame") {
					// check if request is made by the host
					itr = document.FindMember("lobbyId");
					if (itr == document.MemberEnd()) {
						throw std::invalid_argument("bad json object");
					}
					lobbyID = std::stoi(document["lobbyId"].GetString());
					Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
					if (clientObj->getPlayer_Id() == lobby->getLobbyOwner() && lobby->getLobbyReady()) {
						lobby->setStatus("active");
						broadcastStartLobby(lobby);
						// send response to all clients to load the game
							
						initializeSync();
						createClientAndUpdateThreads(lobby);
					} else {
							throw std::invalid_argument("Not all players ready");
					}
					
				}
				else if (request == "playerReady") {
					clientObj->setLoadingStatus("true");
					// change the status of player 
					Lobby * lobby = lobbyManager->getLobbyObject(lobbyID);
					if (lobby->getLoadingReady()) {
						// send response to all clients to start the game
						broadcastStartGame(lobby);
					}
				}
           	}
			} catch (...) {
				printf("Catching an exception");
				string errorResponse = "{\"statusCode\":500}";
				send(sockfd, errorResponse.c_str(), errorResponse.size(), 0);
			}
		}                                            
        if (--nready <= 0)
            break;    
			    // no more readable descriptors
        }
		}
	memset(buffer, 0, BUFLEN);
    }
	return(0);
}


// Prints the error stored in errno and aborts the program.
static void SystemFatal(const char* message)
{
    perror (message);
    exit (EXIT_FAILURE);
}