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
void LobbyManager::createLobby(int owner) {
    Lobby * lobby = new Lobby(owner);
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