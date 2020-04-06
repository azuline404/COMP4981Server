/*
 *  NAME:            LobbyManager.cpp
 *  DESC:            This class 
 *  DESIGNER:        Ash Ketchum
 *  PROGRAMMER:      Pikachu
 *  REVISIONS:       Nicole J &Chirag F (March 10, 2020):
 *						- getlobyList() &getLobby() functionality implemented
 */
#include "LobbyManager.hpp"
#include <iostream>

/*
 *   NAME:            getLobbyList()
 *   DESC:            This function returns a string of the lobby list information in 
 *					  json format
 *   DESIGNER:        Ash Ketchum
 *   PROGRAMMER:      Nicole J & Chirag F
 *   REVISIONS:       Tommy Chang 3/13/2020
 * 						- changed lobbyId and userId to strings, and character class key
 */
string LobbyManager::getLobbyList()
{
	// start of the json string
	string lobbyListJSON = "{\"statusCode\":200";
	lobbyListJSON += ",\"Lobbies\":[";

	// Iterate through the lobby list and resturn a string in json format
	
	for (auto it = lobbyList.begin(); it != lobbyList.end(); it++)
	{
		lobbyListJSON += "{\"lobbyId\":\"" + to_string((*it)->getId()) + "\","+ "\"lobbyStatus\":\"" + (*it)->getStatus() + "\"," +
			"\"lobbyOwnerId\":\"" + to_string((*it)->getLobbyOwner()) + "\"," +
			"\"numPlayers\":\"" + to_string((*it)->getCurrentPlayers()) + "\"}";

		// if not the end add a ","
		if (next(it, 1) != lobbyList.end())
			lobbyListJSON += ",";
	}
	// close off the json string
	lobbyListJSON += "]";
	lobbyListJSON += "}";
	return lobbyListJSON;
}

/*
 *   NAME:            getLobby(int id)
 *   DESC:            This function returns the lobby information including the players 
 *					  given the owners id.
 *   DESIGNER:        Ash Ketchum
 *   PROGRAMMER:      Nicole J & Chirag F
 *   REVISIONS:       Nicole J (March 11, 2020):
 *						- Fixed bug with the string in formatter in the loop 	
 *					  Tommy Chang (March 19, 2020)
 *						- adding allPlayersReady field										
 */

string LobbyManager::getLobby(int id)
{
	string allPlayersReady;

	// Start the json string
	string lobbyJSON = "{\"statusCode\":200, \"responseType\":\"modify\"";

	// Start of "Players" key of player array
	lobbyJSON += ",\"Players\":[";

	Lobby * currentLobby = getLobbyObject(id);

	// Make a copy of the current client list
	vector<Client*> currentClientList = currentLobby->getClientList();
	// Iterate through the client list
	for (auto it = currentClientList.begin(); it != currentClientList.end(); it++)
	{
		// create a string of the client information in json format 
		lobbyJSON += "{\"userId\":\"" + to_string((*it)->getPlayer_Id()) + "\"," +
			"\"classType\":\"" + to_string((*it)->getCharacterClass()) + "\"," +
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

	int lobbyOwner = currentLobby->getLobbyOwner();
	string ownerName;

	for (auto it = currentClientList.begin(); it != currentClientList.end(); it++) {
		if ((*it)->getPlayer_Id() == lobbyOwner) {
			ownerName = (*it)->getPlayer_name();
			break;
		} 
	}

	allPlayersReady = currentLobby->getLobbyReady()? "true" : "false";
	// Rest of the lobby information
	lobbyJSON += "\"lobbyId\":\"" + to_string(currentLobby->getId()) + "\"," + 
		"\"lobbyStatus\":\"" + currentLobby->getStatus() + "\"," +
		"\"allPlayersReady\":\"" + allPlayersReady + "\"," + "\"lobbyOwnerName\":\"" + ownerName + 
		"\"" + ",\"lobbyOwnerId\":\"" + to_string(lobbyOwner) + "\"" + ",\"numPlayers\":\"" 
		+ to_string(currentLobby->getCurrentPlayers()) + "\"" +
		"}";
	return lobbyJSON;
}

/*
 *   NAME:            deleteLobby(int lobbyId)
 *   DESC:            This function deletes a lobby from the lobbyList
 *   DESIGNER:        Ash Ketchum
 *   PROGRAMMER:      Tommy C 
 *   REVISIONS:       NA										
 */
void LobbyManager::deleteLobby(int lobbyId)
{
	for (auto it = lobbyList.begin(); it != lobbyList.end(); it++)
	{
		if ((*it)->getId() == lobbyId)
		{
			lobbyList.erase(it);
			free(*it);
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
	numLobbies++;
	return lobby->getId();
}

/*
 *   NAME:            getLobbyObject(int id)
 *   DESC:            This function returns the game lobby object given an id
 *   DESIGNER:        Ash Ketchum
 *   PROGRAMMER:      Tommy C 
 *   REVISIONS:       NA										
 */
Lobby *LobbyManager::getLobbyObject(int id)
{
	for (auto it = lobbyList.begin(); it != lobbyList.end(); it++)
	{
		if ((*it)->getId() == id)
		{
			return (*it);
		}
	}
	return NULL;
}

/*
 *   NAME:            verifyLobbyOwner(Client *client)
 *   DESC:            This function verifies whether the given client owns a lobby or not.
 *   DESIGNER:        Sun Kim
 *   PROGRAMMER:      Sun Kim
 *   REVISIONS:       NA										
 */
int LobbyManager::verifyLobbyOwner(Client *client)
{
    int id = client->getPlayer_Id();
    int clientLobbyId = client->getLobby_Id();
    if (id == clientLobbyId)
        return id;
    else
        return -1;
}