/*
 *   NAME:            LobbyManager.cpp
 *   DESC:            This class 
 *   DESIGNER:        Ash Ketchum
 *   PROGRAMMER:    Pikachu
 *   REVISIONS:        Added more spaces
 *               Nicole J & Chirag F (March 10, 2020) - getlobyList() & getLobby()
 */
#include "LobbyManager.hpp"

string LobbyManager::getLobbyList()
{
	// start of the json string
	string lobbyListJSON = "{";

	// Iterate through the lobby list and resturn a string in json format
	for (auto it = lobbyList.begin(); it != lobbyList.end(); it++)
	{
		lobbyListJSON += "\"" + to_string((*it)->getId()) + "\": {" +
			"\"lobbyStatus\" : \"" + (*it)->getStatus() + "\", " +
			"\"lobbyOwner\" : \"" + to_string((*it)->getLobbyOwner()) + "\", " +
			"\"numPlayers\" : \"" + to_string((*it)->getCurrentPlayers()) + "\"}";

		// if not the end add a ","
		if (next(it, 1) != lobbyList.end())
			lobbyListJSON += ",";
	}
	// close off the json string
	lobbyListJSON += "}";
	return lobbyListJSON;
}

string LobbyManager::getLobby(int id)
{
	// Start the json string
	string lobbyJSON = "{";

	// Start of "Players" key of player array
	lobbyJSON += "\"Players\":[";

	// Make a copy of the current client list
	vector<Client*> currentClientList = lobbyList[id]->getClientList();

    // Iterate through the client list
	for (auto it = currentClientList.begin(); it != currentClientList.end(); it++)
	{
        // create a string of the client information in json format 
		lobbyJSON += "{\"id\": \"" + to_string((*it)->getPlayer_Id()) + "\"," +
			"\"class\":\"" + (*it)->getCharacterClass() + "\"," +
			"\"ready\":\"" + (*it)->getStatus() + "\"," +
			"\"username\":\"" + (*it)->getPlayer_name() + "\"," +
			"\"team\":\"" + to_string((*it)->getTeam()) + "\"" +
			+"}";

        // include a "," id its not the end of the client list
		if (next(it, 1) != currentClientList.end())
			lobbyJSON += ",";
	}
    
    // End of the player list array
	lobbyJSON += "],";

    // Rest of the lobby information
	lobbyJSON += "\"lobbyID\":\"" + to_string(lobbyList[id]->getId()) + "\"," +
		"\"lobbyStatus\":\"" + lobbyList[id]->getStatus() + "\"," +
		"\"numPlayers\":\"" + to_string(lobbyList[id]->getCurrentPlayers()) + "\"" +
		"}";
    
	return lobbyJSON;
}

void LobbyManager::deleteLobby(int lobbyId)
{

	for (auto it = lobbyList.begin(); it != lobbyList.end(); it++)
	{
		if ((*it)->getId() == lobbyId)
		{
			int numPlayers = (*it)->getCurrentPlayers();
			vector<Client*> list = (*it)->getClientList();
			for (int i = 0; i < numPlayers; i++)
			{
				list[i]->setLobby_Id(0);
			}
			lobbyList.erase(it);
			break;
		}
	}
}

int LobbyManager::createLobby(Client *client)
{
	int id = client->getPlayer_Id();
	Lobby *lobby = new Lobby(id);
	lobby->addClient(client);
	client->setLobby_Id(lobby->getId());
	lobbyList.push_back(lobby);
	return lobby->getId();
}

Lobby *LobbyManager::getLobbyObject(int id)
{
	return lobbyList[id];
}