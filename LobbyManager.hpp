#include "Lobby.hpp"

using namespace std;
class LobbyManager {
    public:
        LobbyManager() = default;
        int createLobby(Client * client);
        string getLobby(int id);
        string getLobbyList();
        void deleteLobby(int lobbyId);
        Lobby * getLobbyObject(int id);
        int verifyLobbyOwner(Client* client);
        std::vector<Lobby*>lobbyList;
    private:
        // std::vector<Lobby*>lobbyList;
        int numLobbies = 0;
};