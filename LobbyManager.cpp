#include "LobbyManager.hpp"

string LobbyManager::getLobbyList() {
    string lobbyListJSON = "";
    for (auto it = lobbyList.begin(); it != lobbyList.end(); it++) {
            // TO-DO:
            // Convert lobby into string
            // return it 
    }
    return lobbyListJSON;
}

void LobbyManager::deleteLobby(int lobbyId) {
    
    for (auto it = lobbyList.begin(); it != lobbyList.end(); it++) {
        if ((*it)->getId() == lobbyId) {
            int numPlayers = (*it)->getCurrentPlayers;
            vector<Client*> list = (*it)->getClientList();
            for (int i = 0; i < numPlayers; i++) {
                list[i]->setLobby_Id(0);
            }
            lobbyList.erase(it);
            break;
        }
    }
}
void LobbyManager::createLobby(Client * client) {
    int id = client->getPlayer_Id();
    Lobby * lobby = new Lobby(id);
    client->setLobby_Id(lobby->getId());
    lobbyList.push_back(lobby);
}

string LobbyManager::getLobby(int id) {
    for (auto it = lobbyList.begin(); it != lobbyList.end(); it++) {
        // if (it->getID() == id) {
        //     // TO-DO:
        //     // Convert lobby into string
        //     // return it 
        //     string lobbyJSON = "";
        //     return lobbyJSON;
        // }
    }
    return NULL;
}

Lobby* LobbyManager::getLobbyObject(int id) {
    return lobbyList[id];
}
