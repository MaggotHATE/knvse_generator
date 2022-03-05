#pragma once
#include <filesystem>
#include "GameForms.h"
#include "json.h"
#include <stdexcept>

using namespace std;

void Log1(const string& msg);
string GetCurPath();

struct weaponsBank {
	map<int, map<string, string>> formAndFolder;
	map<string, vector<int>> unassigned;
};

struct typesBank {
	map <string, vector<string>> weapType;
	map<string, vector<int>> typesMap;
	vector<string> typesMap1;
	vector<vector<int>> typesMap2;
	vector<string> typesMap3;
	vector<string> excluded;
	vector<string> options;
		

	bool folderInMod(string fileModName, string type) {
		if (!weapType[fileModName].empty()) {
			return (find(weapType[fileModName].begin(), weapType[fileModName].end(), type) != weapType[fileModName].end());
		}
		else if(!weapType["GLOBAL"].empty()) {
			return (find(weapType["GLOBAL"].begin(), weapType["GLOBAL"].end(), type) != weapType["GLOBAL"].end());
		}
		else Log1(" No defines for " + fileModName);
	}

	bool hasFolder(string type) {
		try {
			if (!typesMap[type].empty()) {
				return true;
			}
		}
		catch (const out_of_range& oor) {
			return false;
		}
		return false;
	}
};

struct weaponType {
	UInt32 ID;
	UInt8 modIDX;
	UInt32 modIDXshort;

	string fileModName;
	string name;
	string hexedID;
	vector<int> typeParams;
	string typeFolder;

	bool isThisType(vector<int>& typesData) // -1 is "any" for better flexibility
	{
		if (
			(typeParams[0] == typesData[0] || typesData[0] == -1) &&
			(typeParams[1] == typesData[1] || typesData[1] == -1) &&
			(typeParams[2] == typesData[2] || typesData[2] == -1) &&
			(typeParams[3] == typesData[3] || typesData[3] == -1)
			) {

			return true;
		}
		return false;
	}

	string checkFolders(typesBank& definedTypes)
	{
		//vector<string> matching;
		for (auto type : definedTypes.typesMap) {
			Log1(" Checking for " + fileModName);
			if (isThisType(type.second ) && definedTypes.folderInMod(fileModName, type.first) == true) {
					return type.first;
			}
		}
		return "";
	}

	vector<string> checkFolders1(typesBank& definedTypes)
	{
		vector<string> matching;
		if (!definedTypes.weapType[fileModName].empty()) {
			for (auto type : definedTypes.weapType[fileModName]) {
				Log1(" Checking type " + type);
				if (definedTypes.hasFolder(type) == true) {
					if (isThisType(definedTypes.typesMap[type])) {
						matching.push_back(type);
					}
				}
			}
		}
		else {
			for (auto type : definedTypes.weapType["GLOBAL"]) {
				Log1(" Checking global type " + type);
				if (definedTypes.hasFolder(type) == true) {
					if (isThisType(definedTypes.typesMap[type])) {
						matching.push_back(type);
					}
				}
			}
		}
		return matching;
	}

	vector<string> checkFolders2(typesBank& definedTypes)
	{
		vector<string> matching;
		vector<string>& weaptype = (!definedTypes.weapType[fileModName].empty()) ? definedTypes.weapType[fileModName] : definedTypes.weapType["GLOBAL"];

		for ( auto type : weaptype) {
			Log1(" Checking type " + type);
			for (int i = 0; i < definedTypes.typesMap1.size(); i++) {
				if (definedTypes.typesMap1[i] == type && isThisType(definedTypes.typesMap2[i])) {
						matching.push_back(type);
				}
			}
		}

		return matching;
	}

	vector<string> checkFolders3(typesBank& definedTypes)
	{
		vector<string> matching;

		Log1(" Checking all types ");
		for (int i = 0; i < definedTypes.typesMap1.size(); i++) {
			if (isThisType(definedTypes.typesMap2[i])) {
				matching.push_back(definedTypes.typesMap1[i]);
			}
		}

		return matching;
	}

	vector<string> checkFoldersFile(typesBank& definedTypes, string filename)
	{
		vector<string> matching;
		string fullpath = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types\)" + filename;

		Log1(" Checking types if file: " + fullpath);
		if (filesystem::exists(fullpath))
		{
			for (int i = 0; i < definedTypes.typesMap1.size(); i++) {
				if (isThisType(definedTypes.typesMap2[i]) && definedTypes.typesMap3[i] == fullpath) {
					matching.push_back(definedTypes.typesMap1[i]);
				}
			}
		}

		return matching;
	}

	vector<string> checkFoldersFile1(typesBank& definedTypes, string filename)
	{
		vector<string> matching;

		Log1(" Checking types if file: " + filename);
		vector<string>& weaptype = (!definedTypes.weapType[fileModName].empty()) ? definedTypes.weapType[fileModName] : definedTypes.weapType["GLOBAL"];

		for (auto type : weaptype) {
			if (filesystem::exists(GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types\)" + filename))
			{
				for (int i = 0; i < definedTypes.typesMap1.size(); i++) {
					if (definedTypes.typesMap1[i] == type && isThisType(definedTypes.typesMap2[i]) && definedTypes.typesMap3[i] == filename) {
						matching.push_back(definedTypes.typesMap1[i]);
					}
				}
			}
		}

		return matching;
	}

	map<string, string> getDB(typesBank& definedTypes) {
		map<string, string> FFpairForMod;
		if (find(definedTypes.excluded.begin(), definedTypes.excluded.end(), fileModName) == definedTypes.excluded.end()) {
			FFpairForMod[hexedID] = typeFolder;
		}
		return FFpairForMod;
	}
};

//weaponsBank processTypesMod1(vector<string>& weapType, map<string, vector<int>>& typesMap, vector<string> excluded, string modID = "FF");
//bool processTypesAndWrite(string prefix, string modIDX = "FF");
bool processTypesAndWrite2(string prefix, string modIDX = "FF", int eWeaponType = 0,  int handGrip = 0, int reloadAnim = 0, int attackAnim = 0, int reversed = 0, string typesfilename = "_", string weapsfilename = "_");

typesBank readTypes(vector<int> params, string typesfilename = "_", string weapsfilename = "_");
weaponsBank processTypesMod2(typesBank& definedType, string modID);
weaponType getWeaponData1(TESObjectWEAP* weap);
vector<string> matchAWeapon(TESObjectWEAP* weap);
vector<string> matchAWeapon1(weaponType& weaps, vector<int> params = {});
nlohmann::json writeAweapon(string prefix, TESObjectWEAP* weap, int reversed = 0, string folder = "_");

//template <class num, class str>
//bool processTypesAndWrite3(string prefix, str modIDX, num eWeaponType, num handGrip, num reloadAnim, num attackAnim, num reversed, str filename) {
//	return processTypesAndWrite2(prefix, modIDX, eWeaponType, handGrip, reloadAnim, attackAnim, reversed, filename);
//}
// TBD: return of MAP{folder-params}
// TBD: return of LISTS{folders} per mod
// NOTE: keep reading jsons every time to keep things dynamic - it's not intended to be working in runtime.  
