#include "Client.hpp"
#include <vector>

using namespace std;
class Lobby {
    public:
        Lobby(int lobbyOwner);
        static int lobbyID;
        string getLobbyInfo();
        int getCurrentPlayers();
        string getStatus();
        void setStatus(string status);
        std::vector<Client*> getClientList();
        int getLobbyOwner();
        void setLobbyOwner(int lobbyOwner);
        int getId();
        void setId(int id);
        void addClient(Client* client);
        void removeClient(Client *client);
        void removeAllClients();
        void printInfo();
        bool getLobbyReady();
        bool getLoadingReady();

    private:
        int id;
        int currentPlayers = 0;
        string status;
        std::vector<Client*> clientList;
        int lobbyOwner;
};