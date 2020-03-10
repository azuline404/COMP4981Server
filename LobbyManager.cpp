#include "LobbyManager.hpp"

string LobbyManager::getLobbyList() {

    string lobbyListJSON = "{";

    for (auto it = lobbyList.begin(); it != lobbyList.end(); it++) {
            // TO-DO:
            // Convert lobby into string
            // return it 

            lobbyListJSON += "\"" + to_string((*it)->getId()) +  "\": {" +
            "\"lobbyStatus\" : \"" +  (*it)->getStatus() + "\", " +
            "\"lobbyOwner\" : \"" + to_string((*it)->getLobbyOwner()) + "\", " +
            "\"numPlayers\" : \"" + to_string((*it)->getCurrentPlayers()) = "\"}";

            // if not the end add a ","
            if (next(it, 1) != lobbyList.end())
                lobbyListJSON += ",";
    }
    lobbyListJSON += "}";
    return lobbyListJSON;
}

void LobbyManager::deleteLobby(int lobbyId) {
    
    for (auto it = lobbyList.begin(); it != lobbyList.end(); it++) {
        if ((*it)->getId() == lobbyId) {
            int numPlayers = (*it)->getCurrentPlayers();
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
    lobby->addClient(client);
    client->setLobby_Id(lobby->getId());
    lobbyList.push_back(lobby);
}

string LobbyManager::getLobby(int id) {

        string lobbyJSON = "{";

        lobbyJSON += "\"Players\": [";

        vector<Client*> currentClientList = lobbyList[id]->getClientList();


    for (auto it = currentClientList.begin(); it != currentClientList.end(); it++) {
        // if (it->getID() == id) {
        //     // TO-DO:
        //     // Convert lobby into string
        //     // return it 

        // std::cout << to_string((*it)->getPlayer_Id()) << std::endl;

        lobbyJSON += "{\"id\": \"" + to_string((*it)->getPlayer_Id()) + "\"" +
                        "\"class\":\"" + (*it)->getCharacterClass() + "\"," +
                        "\"ready\":\"" + (*it)->getStatus() + "\"," +
                        "\"name\":\"" + (*it)->getPlayer_name() + "\"," +
                        "\"team\":\"" + to_string((*it)->getTeam()) + "\"" +
                        + "}";

         if (next(it, 1) != currentClientList.end())
                lobbyJSON += ",";                      
        }

        lobbyJSON += "],";

        lobbyJSON += "\"lobbyID\":\"" + to_string(lobbyList[id]->getId()) + "\"," +
                    "\"lobbyStatus\":\"" + lobbyList[id]->getStatus() + "\"," +
                    "\"numPlayers\":\"" + to_string(lobbyList[id]->getCurrentPlayers()) + "\"" +
                    "}";

    return lobbyJSON;
}

Lobby* LobbyManager::getLobbyObject(int id) {
    return lobbyList[id];
}
