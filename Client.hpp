#include <string>

using namespace std;
class Client {
    public:
        static int playerID;
        Client(string characterClass, int team, int socket, int UDPPort, int IP);
        string getPlayer_name();
        void setPlayer_name(string player_name);
        int getPlayer_Id();
        void setPlayer_Id(int player_id);
        string getStatus();
        void setStatus(string status);
        string getCharacterClass();
        void setCharacterClass(string characterClass);
        int getTeam();
        int getTCPSocket();
        void setTeam(int team);
        int getUDPPort();
        void setLobby_Id(int lobbyID);
        int getLobby_Id();
        
    private:
        string player_name;
        int player_id;
        string status;
        string characterClass;
        int team;
        int TCPSocket;
        int UDPPort;
        int IP;
        int lobby_id;
};