#include <string>

using namespace std;
class Client {
    public:
        static int playerID;
        Client(int characterClass, int team, int socket, int UDPPort, int IP);
        string getPlayer_name();
        void setPlayer_name(string player_name);
        int getPlayer_Id();
        void setPlayer_Id(int player_id);
        string getStatus();
        void setStatus(string status);
        int getCharacterClass();
        void setCharacterClass(int characterClass);
        int getTeam();
        int getTCPSocket();
        void setTeam(int team);
        int getUDPPort();
        void setLobby_Id(int lobbyID);
        int getLobby_Id();
        void setLoadingStatus(string newStatus);
        string getLoadingStatus();
        struct sockaddr_in* getUdpAddress();
        void setUDPAddress(sockaddr_in* address);
        void setUDPSocket(int udpSocket);
        int getUDPSocket();
        
    private:
        string player_name;
        int player_id;
        string status;
        int characterClass;
        int team;
        int TCPSocket;
        int UDPSocket;
        int UDPPort;
        int IP;
        int lobby_id;
        string loadingStatus;
        struct sockaddr_in* udpAddress;
};