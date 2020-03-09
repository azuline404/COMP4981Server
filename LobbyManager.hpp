#include "Lobby.hpp"
#include "Client.hpp"

class LobbyManager {
    public:
        int createLobby(int owner);
        string getLobby(int id);
        string getLobbyList();
    private:
        std::vector<Lobby>LobbyList;
};