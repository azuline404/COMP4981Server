class Client {
    public:
        static int playerID;
        Client();
        string getPlayer_name();
        void setPlayer_name(string player_name);
        int getPlayer_id();
        void setPlayer_id(int player_id);
        string getStatus();
        void setStatus(string status);
        string getCharacterClass();
        void setCharacterClass(string characterClass);
        int getTeam();
        void setTeam(int team);
    private:
        string player_name;
        int player_id;
        string status;
        string characterClass;
        int team;
        int TCPSocket;
        int UDPPort;
        int IP;
}