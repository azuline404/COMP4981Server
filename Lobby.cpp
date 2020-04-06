#include "Lobby.hpp"
#include <iostream>

int Lobby::lobbyID = -1;

Lobby::Lobby(int lobbyOwner) {
    this->id = ++lobbyID;
    this->lobbyOwner = lobbyOwner;
    this->status = "inactive";
}

int Lobby::getCurrentPlayers() 
{
    return clientList.size();
}

string Lobby::getStatus()
{
    return this->status;
}

std::vector<pthread_t> Lobby::getThreadIds() {
    return this->threadIds;
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
    client->setLobby_Id(lobbyID);
    if (currentPlayers == 0) {
        this->lobbyOwner = client->getPlayer_Id();
    }
    this->currentPlayers++;
}

void Lobby::removeClient(Client *client)
{
    for (auto it = clientList.begin(); it != clientList.end(); it++) {
        if ((*it)->getPlayer_Id() == client->getPlayer_Id()) {
            clientList.erase(it);
            break;
        }
    }
    client->setCharacterClass(0);
    client->setLobby_Id(-1);
    client->setTeam(0);
    client->setStatus("false");
    currentPlayers--;
    if (currentPlayers == 0) {
        this->setLobbyOwner(-1);
    }
}

void Lobby::removeAllClients()
{
    auto iter = clientList.begin();
    for ( ; iter != clientList.end(); iter++)
        (*iter)->setLobby_Id(-1);
    clientList.clear();
    this->currentPlayers = 0;
}

void Lobby::printInfo() {
    cout << "current players:" << currentPlayers << endl;
    cout << "status:" << currentPlayers << endl;
    cout << "lobbyOwner:" << currentPlayers << endl;
}

bool Lobby::getLobbyReady() {
    for (auto it = clientList.begin(); it != clientList.end(); it++) {
        if ((*it)->getStatus() == "false") {
            return false;
        }
    }
    return true;
}
bool Lobby::getLoadingReady() {
    for (auto it = clientList.begin(); it != clientList.end(); it++) {
        if ((*it)->getLoadingStatus() == "false") {
            return false;
        }
    }
    return true;
}