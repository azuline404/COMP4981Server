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

#define PARENTID "parentID"
#define TEAM "team"
#define TYPE "type"
#define HEALTH "health"
#define DIRECTION "direction"
#define POSITION "position"
#define STATE "state"

using namespace rapidjson;
using namespace std;

#define READ_BUFF 65536

string updateJSONFile(string clientObj);

string modifyJSONObject(Document &file, Document &client, string id);
string addToJSONObject(Document &file, Document &client, string id);

string getUpdatedJSONFile();
void writeFile(const char *json);
void copyFromDefaultFile();