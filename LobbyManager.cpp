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
int LobbyManager::createLobby(int owner) {
    Lobby lobby = new Lobby(owner);
    lobbyList.push(lobby);
}

Lobby LobbyManager::getLobby(int id) {
    for (auto it = lobbyList.begin(); it != lobbyList.end(); it++) {
        if (it->getID() == id) {
            // TO-DO:
            // Convert lobby into string
            // return it 
            string lobbyJSON = "";
            return lobbyJSON;
        }
    }
    return NULL;
}