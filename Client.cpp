#include "Client.hpp"

int Client::playerID = 0;
Client::Client(int characterClass, int team, int socket, int UDPPort, int IP){
    this->player_id = playerID++;
    this->characterClass = characterClass;
    this->status = "false";
    this->team = team;
    this->TCPSocket = socket;
    this->UDPPort = UDPPort;
    this->IP = IP;
    this->lobby_id = -1;
    this->loadingStatus = "false";
}
void Client::setLobby_Id(int lobbyID) {
    this->lobby_id = lobbyID;
}
int Client::getLobby_Id() {
    return this->lobby_id;
}

int Client::getUDPPort() {
    return this->UDPPort;
}

int Client::getTCPSocket() {
    return this->TCPSocket;
}

string Client::getPlayer_name()
{
    return this->player_name;
}

void Client::setPlayer_name(string player_name)
{
    this->player_name = player_name;
}


int Client::getPlayer_Id()
{
    return this->player_id;
}

void Client::setPlayer_Id(int player_id)
{
    this->player_id = player_id;
}


string Client::getStatus()
{
    return this->status;
}

void Client::setStatus(string status)
{
    this->status = status;
}


int Client::getCharacterClass()
{
    return this->characterClass;
}


void Client::setCharacterClass(int characterClass)
{
    this->characterClass = characterClass;
}


int Client::getTeam()
{
    return this->team;
}

void Client::setTeam(int team)
{
    this->team = team;
}

void Client::setLoadingStatus(string newStatus)
{
    this->loadingStatus = newStatus;
}

string Client::getLoadingStatus()
{
    return this->loadingStatus;
}

struct sockaddr_in* Client::getUdpAddress() {
    return this->udpAddress;
}