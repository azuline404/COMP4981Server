/*
*	NAME:			GameState
*	DESC:			GameState is used to update the main Gamestate JSON File
*	DESIGNER:		Nicole Jingco
*	PROGRAMMER:	    Nicole Jingco
*	REVISIONS:		
*/

// rapidjson/example/simpledom/simpledom.cpp`
// This file creates the defualt json file/overwrite the defualt json file
#include "GameState.hpp"

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
   
    // Write to JSON File
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    file.Accept(writer);
    writeFile(buffer.GetString());
    return update;
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
    // Get Object from file with key of id
    Value &fileInfo = file[id.c_str()];
    assert(fileInfo.IsObject());

    // Get Object of client 
    Value &clientInfo = client[id.c_str()];
    assert(clientInfo.IsObject());

    // Update ParentID Value
    if (clientInfo.HasMember(PARENTID)){
        assert(clientInfo[PARENTID].IsInt());
        fileInfo[PARENTID] = clientInfo[PARENTID];
    }

    // Update Team Value
    if (clientInfo.HasMember(TEAM)){
        assert(clientInfo[TEAM].IsInt());
        fileInfo[TEAM] = clientInfo[TEAM];
    }

    // Update Type Value
    if (clientInfo.HasMember(TYPE)){
        assert(clientInfo[TYPE].IsInt());
        fileInfo[TYPE] = clientInfo[TYPE];
    }
    // Update Health Value
    if (clientInfo.HasMember(HEALTH)){
        assert(clientInfo[HEALTH].IsInt());
        fileInfo[HEALTH] = clientInfo[HEALTH];
    }

    // Update State Value
    if (clientInfo.HasMember(STATE)){
        assert(clientInfo[STATE].IsInt());
        fileInfo[STATE] = clientInfo[STATE];
    }

    // Update Position Value
    if (clientInfo.HasMember(POSITION)){
        assert(clientInfo[POSITION].IsObject());
        fileInfo[POSITION] = clientInfo[POSITION]; 
    }

    // Update Direction Value
    if (clientInfo.HasMember(DIRECTION)){
        assert(clientInfo[DIRECTION].IsObject());
        fileInfo[DIRECTION] = clientInfo[DIRECTION]; 
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
    if (clientInfo.HasMember(PARENTID)){
        assert(clientInfo[PARENTID].IsInt());
        parentIDVal = clientInfo[PARENTID].GetInt();
    }
    if (clientInfo.HasMember(TEAM)){
        assert(clientInfo[TEAM].IsInt());
        teamVal = clientInfo[TEAM].GetInt();
    }
    if (clientInfo.HasMember(TYPE)){
        assert(clientInfo[TYPE].IsInt());
        typeVal = clientInfo[TYPE].GetInt();
    }
    if (clientInfo.HasMember(HEALTH)){
        assert(clientInfo[HEALTH].IsInt());
        healthVal = clientInfo[HEALTH].GetInt();
    }

    if (clientInfo.HasMember(STATE)){
        assert(clientInfo[STATE].IsInt());
        stateVal = clientInfo[STATE].GetInt();
    } 

    if (clientInfo.HasMember(POSITION)){
        assert(clientInfo[POSITION].IsObject());
        positionVal = clientInfo[POSITION];
    }

    if (clientInfo.HasMember(DIRECTION)){
        assert(clientInfo[DIRECTION].IsObject());
        directionVal = clientInfo[DIRECTION];
    }

    // Add values to temprary object
    elements.AddMember(StringRef(PARENTID), parentIDVal, allocator);
    elements.AddMember(StringRef(TEAM), teamVal, allocator);
    elements.AddMember(StringRef(TYPE), typeVal, allocator);
    elements.AddMember(StringRef(HEALTH), healthVal, allocator);
    elements.AddMember(StringRef(STATE), stateVal, allocator);
    elements.AddMember(StringRef(POSITION), positionVal, allocator);
    elements.AddMember(StringRef(DIRECTION), directionVal, allocator);

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

