#include "Lobby.hpp"

using namespace std;
class LobbyManager {
    public:
        LobbyManager() = default;
        void createLobby(int owner);
        string getLobby(int id);
        string getLobbyList();
    private:
        std::vector<Lobby*>lobbyList;
};