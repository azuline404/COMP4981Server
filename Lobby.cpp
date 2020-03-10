#include "Lobby.hpp"

int Lobby::lobbyID = 0;

Lobby::Lobby(int lobbyOwner) {
    this->id = lobbyID++;
    this->lobbyOwner = lobbyOwner;
    this->status = "inactive";
}

int Lobby::getCurrentPlayers() 
{
    return this->currentPlayers;
}

string Lobby::getStatus()
{
    return this->status;
}


void Lobby::setStatus(string status)
{
    this->status = status;
}


std::vector<Client*> Lobby::getClientList()
{
    return this->clientList;
}


int Lobby::getLobbyOwner()
{
    return this->lobbyOwner;
}

void Lobby::setLobbyOwner(int lobbyOwner)
{
    this->lobbyOwner = lobbyOwner;
}

int Lobby::getId()
{
    return this->id;
}

void Lobby::setId(int id)
{
    this->id = id;
}

void Lobby::addClient(Client * client) {
    clientList.push_back(client);
    this->currentPlayers++;
}
