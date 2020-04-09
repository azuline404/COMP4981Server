/*
*	NAME:			GameState
*	DESC:			GameState is used to update the main Gamestate JSON File
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		
*/

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
#include <sstream>
#include <algorithm>

using namespace rapidjson;
using namespace std;

#define READ_BUFF 65536

string updateJSONFile(string clientObj);

string modifyJSONObject(Document &file, Document &client, string id);
string addToJSONObject(Document &file, Document &client, string id);

string getUpdatedJSONFile();
void writeFile(const char *json);
void copyFromDefaultFile();

/*
*	NAME:			updateJSONFile
*	DESC:			updateJSONFile is used modify the json file with the 
*                   string recieved from the client and returns the client info update
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		NA
*/ 
string updateJSONFile(string clientObj)
{
    const char *  obj = clientObj.c_str();
    string update;
    // get JSON key
    string id;
    stringstream ss(clientObj);
    ss >> id;
    id.erase(0,2);
    id.erase(id.length()-2, 3);

    // Open JSON and add to document object
    FILE *fp = fopen("GameState.json", "rb"); // non-Windows use "w"
    char readBuffer[READ_BUFF];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document file, // JSON File
             client;// client temp

    file.ParseStream(is);
    assert(file.IsObject());
    
    client.Parse(obj);
    assert(client.IsObject());

    // Check if id exist 
    if (file.HasMember(id.c_str()))
        update = modifyJSONObject(file, client, id);
    else
        update  = addToJSONObject(file, client, id);
   
    cout << update << endl;

    // Write to JSON File
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    file.Accept(writer);
    writeFile(buffer.GetString());
}

/*
*	NAME:			modifyJSONObject
*	DESC:			modifyJSONObject is used to update  teh JSON File with
*                   an existing id and returns the object
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		NA
*/ 
string modifyJSONObject(Document &file, Document &client, string id)
{ 
    const char * parentID = "parentID";
    const char * team = "team";
    const char * type = "type";
    const char * health = "health";
    const char * direction = "direction";
    const char * position = "position";
    const char * state = "state";

    // Get Object from file with key of id
    Value &fileInfo = file[id.c_str()];
    assert(fileInfo.IsObject());

    // Get Object of client 
    Value &clientInfo = client[id.c_str()];
    assert(clientInfo.IsObject());

    // Update Health Value
    if (clientInfo.HasMember(parentID)){
        assert(clientInfo[parentID].IsInt());
        fileInfo[parentID] = clientInfo[parentID];
    }

    // Update Health Value
    if (clientInfo.HasMember(team)){
        assert(clientInfo[team].IsInt());
        fileInfo[team] = clientInfo[team];
    }

    // Update Health Value
    if (clientInfo.HasMember(type)){
        assert(clientInfo[type].IsInt());
        fileInfo[type] = clientInfo[type];
    }
    // Update Health Value
    if (clientInfo.HasMember(health)){
        assert(clientInfo[health].IsInt());
        fileInfo[health] = clientInfo[health];
    }

    // Update State Value
    if (clientInfo.HasMember(state)){
        assert(clientInfo[state].IsInt());
        fileInfo[state] = clientInfo[state];
    }

    // Update Position Value
    if (clientInfo.HasMember(position)){
        assert(clientInfo[position].IsObject());
        fileInfo[position] = clientInfo[position]; 
    }

    // Update Direction Value
    if (clientInfo.HasMember(direction)){
        assert(clientInfo[direction].IsObject());
        fileInfo[direction] = clientInfo[direction]; 
    }

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    fileInfo.Accept(writer);
    return "{\"" + id + "\": " + sb.GetString() + "}";

}

/*
*	NAME:			addToJSONObject
*	DESC:			addToJSONObject is used to add a NEW Object to the JSON 
*                   file. Value will have default values(0, {}) if updates 
*                   received does not include those elements and returns the object
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		NA
*/ 
string addToJSONObject(Document &file, Document &client, string id)
{
    const char * parentID = "parentID";
    const char * team = "team";
    const char * type = "type";
    const char * health = "health";
    const char * direction = "direction";
    const char * position = "position";
    const char * state = "state";
  

    int parentIDVal = 0;
    int teamVal = 0;
    int typeVal = 0;
    int healthVal = 0;
    int stateVal = 0;
    Value positionVal(kObjectType);
    Value directionVal(kObjectType);

    Document::AllocatorType& allocator = file.GetAllocator();
    
    // Temporary value to store temporary Object
    Value elements;
    elements.SetObject();

    Value &clientInfo = client[id.c_str()];
    assert(clientInfo.IsObject());
    
    // Get Values from client
    if (clientInfo.HasMember(parentID)){
        assert(clientInfo[parentID].IsInt());
        parentIDVal = clientInfo[parentID].GetInt();
    }
    if (clientInfo.HasMember(team)){
        assert(clientInfo[team].IsInt());
        teamVal = clientInfo[team].GetInt();
    }
    if (clientInfo.HasMember(type)){
        assert(clientInfo[type].IsInt());
        typeVal = clientInfo[type].GetInt();
    }
    if (clientInfo.HasMember(health)){
        assert(clientInfo[health].IsInt());
        healthVal = clientInfo[health].GetInt();
    }

    if (clientInfo.HasMember(state)){
        assert(clientInfo[state].IsInt());
        stateVal = clientInfo[state].GetInt();
    } 

    if (clientInfo.HasMember(position)){
        assert(clientInfo[position].IsObject());
        positionVal = clientInfo[position];
    }

    if (clientInfo.HasMember(direction)){
        assert(clientInfo[direction].IsObject());
        directionVal = clientInfo[direction];
    }

    // Add values to temprary object
    elements.AddMember(StringRef(parentID), parentIDVal, allocator);
    elements.AddMember(StringRef(team), teamVal, allocator);
    elements.AddMember(StringRef(type), typeVal, allocator);
    elements.AddMember(StringRef(health), healthVal, allocator);
    elements.AddMember(StringRef(state), stateVal, allocator);
    elements.AddMember(StringRef(position), positionVal, allocator);
    elements.AddMember(StringRef(direction), directionVal, allocator);

    // Add object to Main Object from file
    file.AddMember(StringRef(id.c_str()), elements, allocator);    // not working
    
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    file[id.c_str()].Accept(writer);
    return "{\"" + id + "\": " + sb.GetString() + "}";
}

/*
*	NAME:			getUpdatedJSONFile
*	DESC:			getUpdatedJSONFile Reads JSON file and returns the file in string
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		NA
*/ 
string getUpdatedJSONFile()
{
    FILE *fp = fopen("GameState.json", "wb"); // non-Windows use "w"
    Document d;
    char readBuffer[READ_BUFF];

    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    d.ParseStream(is);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    fclose(fp);

    return buffer.GetString();
}

/*
*	NAME:			writeFile
*	DESC:			writeFile is used to write the updates on the GameStateFile
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		NA
*/ 
void writeFile(const char *json)
{
    FILE *fp = fopen("GameState.json", "wb"); // non-Windows use "w"
    Document d;
    char writeBuffer[READ_BUFF];

    d.Parse(json);

    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

    Writer<FileWriteStream> writer(os);
    d.Accept(writer);

    fclose(fp);
}

/*
*	NAME:			copyFromDefaultFile
*	DESC:			copyFromDefaultFile is used to copy the default game
*                   to a temporary file
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		NA
*/ 
void copyFromDefaultFile()
{
    FILE *fp = fopen("GameStateDefault.json", "rb"); // non-Windows use "r"
    Document d;
    char readBuffer[READ_BUFF];

    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    d.ParseStream(is);
    fclose(fp);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    writeFile(buffer.GetString());
}

