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