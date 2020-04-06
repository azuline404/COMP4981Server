// rapidjson/example/simpledom/simpledom.cpp`
// This file creates the defualt json file/overwrite the defualt json file
#include "../rapidjson/document.h"
#include "../rapidjson/filereadstream.h"
#include "../rapidjson/filewritestream.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

using namespace rapidjson;
using namespace std;

#define READ_BUFF 65536

// Client Lobby WALP
// void addClientToLobby(string client);
void modifyArray(const char *object, const char *lookupKey, const char *lookupVal, const char *updateKey, string updateVal);
void addToClient(const char *object, const char *json);
void updateFile(Document d);
void writeFile(const char *json);
void defaultGameState();
string getPlayersInfo();
string getCrystalsInfo();
void addInfoToFile();

// 
void modifyObject(string object);
void modifyWorldObject(string obj);

string sendUpdatedObject();



int main()
{
// int main()
// {
//     string client = "{\"123\" : {" 
//                                 "\"health\":44," 
//                                 "\"position\": { " 
//                                     "\"x\": -50.0, "
//                                     "\"y\": 5.37, "
//                                     "\"z\": 69.0 "
//                                 "}, \"direction\": { "
//                                     "\"x\": 5.0, "
//                                     "\"y\": 4.0, "
//                                     "\"z\": 7.7 "
//                                 "}}}";

//     string object = "{\"111\" : {\"health\":77}}";
//     string projectile = "{\"999\" : {\"health\":66}}";
    
//     updateJSONFile(client);
//     updateJSONFile(object);
//     updateJSONFile(projectile);

//     cout << getUpdatedJSONFile() << endl;

    return 0;
}
// ----------------------------------------------------------------------------------

// Use this function 
void modifyClientObject(string *obj)
{
    // only thing that changes:
    string health = "health";
    string direction = "direction";
    string position = "position";
    string state = "state";
    char readBuffer[READ_BUFF];

      // Open JSON and add to document object
    FILE *fp = fopen("GameStateTest.json", "rb");// non-Windows use "r"
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document file, // JSON File
             temp, // temporary JSON
             client; // Clients Information

    file.ParseStream(is);
    assert(file.IsObject());


    client.Parse(obj);
    client.SetObject();




    // gets client object string
    // parse it to json format
    // get each element to be used to update the object

    // open game state file
    // find object, 
    // 
}

// Use this function for world object sent from "Game host?"
void modifyWorldObject(string obj)
{

}

// updated JSON object
string sendUpdatedObject()
{

} 


// ----------------------------------------------------------------------------------

//  Adds client info in json format to the client (player) array
void addToClient(const char *object, const char *json)
{
    // Open JSON and add to document object
    FILE *fp = fopen("output.json", "r");
    char readBuffer[READ_BUFF];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document d, d2, temp;
    d.ParseStream(is);
    assert(d[object].IsArray());
    fclose(fp);

    temp.Parse(json);

    d2.SetObject();
    Value json_objects(kObjectType);

    assert(temp.IsObject());
    Value &playerName = temp["PlayerName"];
    Value &playerClass = temp["PlayerClass"];

    assert(playerName.IsString());
    assert(playerClass.IsString());

    json_objects.AddMember("PlayerName", StringRef(playerName.GetString()), d2.GetAllocator());
    json_objects.AddMember("PlayerClass", StringRef(playerClass.GetString()), d2.GetAllocator());

    // Adds the json created to the end of the array
    d[object].PushBack(json_objects, d2.GetAllocator());

    // Writes updates to the file
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    writeFile(buffer.GetString());
}

// TODO : issue wiht updateVal , issue when assining
void modifyArray(const char *object, const char *lookupKey, const char *lookupVal, const char *updateKey, string updateVal)
{
    FILE *fp = fopen("output.json", "rb"); // non-Windows use "r"
    Document d;
    char readBuffer[READ_BUFF];

    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    d.ParseStream(is);

    assert(d.IsObject());

    assert(d.HasMember(object));
    // object hold as array
    assert(d[object].IsArray());

    Value &players = d[object];

    // Itreate through Players
    for (rapidjson::Value::ValueIterator itr = players.Begin(); itr != players.End(); ++itr)
    {
        rapidjson::Value &attribute = *itr;
        assert(attribute.IsObject()); // each attribute is an object

        for (rapidjson::Value::MemberIterator itr2 = attribute.MemberBegin(); itr2 != attribute.MemberEnd(); ++itr2)
        {
            if (strcmp(itr2->name.GetString(), lookupKey) == 0 && strcmp(itr2->value.GetString(), lookupVal) == 0)
            {
                rapidjson::Value::MemberIterator changeKey = next(itr2, 1);
                if (strcmp(changeKey->name.GetString(), updateKey) == 0)
                {
                    cout << "Pre modify: " << endl;
                    cout << itr2->name.GetString() << " : " << itr2->value.GetString() << endl;
                    cout << changeKey->name.GetString() << " : " << changeKey->value.GetString() << endl;

                    // Modify
                    changeKey->value = StringRef(updateVal.c_str());

                    cout << "Post modify: " << endl;
                    cout << itr2->name.GetString() << " : " << itr2->value.GetString() << endl;
                    cout << changeKey->name.GetString() << " : " << changeKey->value.GetString() << endl;
                }
            }
        }
    }

    Writes updates to the file
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // // cout << buffer.GetString() << endl;
    writeFile(buffer.GetString());

    fclose(fp);
}

// Creates the the json string of the default game state
void defaultGameState()
{
    vector<string> topic = {"Players", "Crystal"};
    string json_obj = "{";
    string jsonPlayerList = getPlayersInfo();
    string jsonCrystalList = getCrystalsInfo();

    for (int t = 0; t < topic.size(); t++)
    {
        switch (t)
        {
        case 0:
            // adds the player json
            json_obj += "\"" + topic[t] + "\" : " + jsonPlayerList;
            break;
        case 1:
            // adds the crystal json
            json_obj += "\"" + topic[t] + "\" : " + jsonCrystalList;
            break;
        default:
            break;
        }

        if ((t + 1) < topic.size())
            json_obj += ",";
    }
    json_obj += "}";

    const char *c = json_obj.c_str();
    writeFile(c);
}

// Creates a json of the player list
string getPlayersInfo()
{
    vector<string> player_name = {"Zafir", "Henry", "Tomas"};
    vector<string> player_class = {"Knight", "Wizard", "Bard"};

    string jsonPlayerList = "[";

    for (int i; i < player_name.size(); i++)
    {
        jsonPlayerList += "{";

        jsonPlayerList += "\"PlayerName\" : \"" + player_name[i] + "\",";
        jsonPlayerList += "\"PlayerClass\" : \"" + player_class[i] + "\"";

        jsonPlayerList += "}";
    }
    jsonPlayerList += "]";

    return jsonPlayerList;
}

// Creates the Crystal Json and returns a string
string getCrystalsInfo()
{
    string jsonCrystalList = "[{\"team\" : 1, \"health\": 500}, {\"team\": 2, \"health\" : 500}]";
    return jsonCrystalList;
}

// Writes a cosnt char * to a json file
void writeFile(const char *json)
{
    FILE *fp = fopen("output.json", "wb"); // non-Windows use "w"
    Document d;
    char writeBuffer[READ_BUFF];

    d.Parse(json);

    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

    Writer<FileWriteStream> writer(os);
    d.Accept(writer);

    fclose(fp);
}

// Reads a json file
void readFile()
{
    FILE *fp = fopen("output.json", "rb"); // non-Windows use "r"
    Document d;
    char readBuffer[READ_BUFF];

    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    d.ParseStream(is);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    cout << buffer.GetString() << std::endl;

    fclose(fp);
}