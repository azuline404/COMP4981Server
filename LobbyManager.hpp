#include "Lobby.hpp"

using namespace std;
class LobbyManager {
    public:
        LobbyManager() = default;
        void createLobby(Client * client);
        string getLobby(int id);
        string getLobbyList();
        void deleteLobby(int lobbyId);
        Lobby * getLobbyObject(int id);
    private:
        std::vector<Lobby*>lobbyList;
};