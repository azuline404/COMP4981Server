#include "Client.hpp"

class Lobby {
    public:
        Lobby();
        static int lobbyID;
        string getLobbyInfo();
        int getCurrentPlayers();
        string getStatus();
        void setStatus(string status);
        std::vector<Client> getClientList();
        void setClientList(std::vector<Client> clientList);
        int getLobbyOwner();
        void setLobbyOwner(int lobbyOwner);

    private:
        int id;
        int currentPlayers = 0;
        string status;
        std::vector<Client> clientList;
        int lobbyOwner;
}