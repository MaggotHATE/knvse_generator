#pragma once
#include <filesystem>
#include "GameForms.h"
#include "json.h"

#include <stdexcept>
#include <thread>
#include <future>
#include <chrono>


using namespace std;

void Log1(const string& msg);
string GetCurPath();



struct aniMap {
	vector<string> Type = {
	"h2h", //0
	"1hm", //1
	"2hm", //2
	"1hp", //3
	"1hp", //4
	"2hr", //5
	"2ha", //6
	"2hr", //7
	"2hh"  //8
	};

	vector<string> Reload = {
	"reloada", //0
	"reloadb", //1
	"reloadc", //2
	"reloadd", //3
	"reloade", //4
	"reloadf", //5
	"reloadg", //6
	"reloadh", //7
	"reloadi", //8
	"reloadj", //9
	"reloadk", //10
	"reloadl", //11
	"reloadm", //12
	"reloadn", //13
	"reloado", //14
	"reloadp", //15
	"reloadq", //16
	"reloadr", //17
	"reloads", //18
	"reloadw", //19
	"reloadx", //20
	"reloady", //21
	"reloadz"  //22
	};

	map<string, int> Attk1 = {
		{"attackleft", 26},
		{"attackright", 32},
		{"attack3" , 38},
		{"attack4" , 44},
		{"attack5" , 50},
		{"attack6" , 56},
		{"attack7" , 62},
		{"attack8" , 68},
		{"attack9" , 144},
		{"attackloop" , 74},
		{"attackspin" , 80},
		{"attackspin2" , 86},
		{"attackthrow" , 114},
		{"attackthrow2" , 120},
		{"attackthrow3" , 126},
		{"attackthrow4" , 132},
		{"attackthrow5" , 138},
		{"attackthrow6" , 150},
		{"attackthrow7" , 156},
		{"attackthrow8" , 162},
		{"placemine" , 102},
		{"placemine2" , 108}
	};

	void scanFiles (const string SfileName0, map<string, map<string, int>>& result) {
		string loclog = "scanFiles "+ SfileName0 + " \n";
		string SfileName;
		transform(SfileName0.begin(), SfileName0.end(), std::back_inserter(SfileName), ::tolower);
		//Log1(" Valid file !");
		int aType = -1;
		for (int i = 0; i < Type.size(); i++) {
			int pos = SfileName.find(Type[i]);
			//Log1(" Found " + Type[i] + " in " + to_string(pos));
			if (pos != string::npos && pos != -1) {
				//Log1(" Has " + Type[i] + " at " + to_string(i) + ", proceeding...");
				loclog += " Has " + Type[i] + " at " + to_string(i) + ", proceeding...\n";
				aType = i;
				int aReload = -1;
				for (int j = 0; j < Reload.size(); j++) {
					int pos1 = SfileName.find(Reload[j]);

					if (pos1 != string::npos) {
						aReload = j;
						//Log1(" Found Reload " + Reload[j] + " in " + to_string(j));
						
						result[Type[i] + to_string(aType)]["Grip"] = -1;
						result[Type[i] + to_string(aType)][Reload[j]] = aReload;
						//Log1("Found reload anim " + SfileName + " for " + to_string(aType) + " -1 " + to_string(aReload) + " -1 ");
						loclog += "Found reload anim " + SfileName + " for " + to_string(aType) + " -1 " + to_string(aReload) + " -1 \n";
					}
				}
				if (aReload == -1) {
					int aAttk = -1;
					auto last = SfileName.length() - 6;
					string subname = SfileName.substr(3, last);
					//Log1(" Searching for " + subname + " between 3 and " + to_string(last));

					try
					{
						//Log1(" Found " + to_string(k));
						if (Attk1.at(subname)) {

							aAttk = Attk1.at(subname);
							//Log1(" Found Attk " + subname + " in " + to_string(aAttk) + " at " + to_string(pos));
							result[Type[i] + to_string(aType)][subname] = aAttk;
							//Log1("Found attack anim " + SfileName + " for " + to_string(aType) + " -1 -1 " + to_string(aAttk));
							loclog += "Found attack anim " + SfileName + " for " + to_string(aType) + " -1 -1 " + to_string(aAttk) + "\n";
						}
					}
					catch (const out_of_range& oor) {
						//Log1("This weapontype wasn't defined: " + weapType[i]);
						//result[Type[i]]["noattacks"]= -2;
					}
				}
			}
		}
		Log1(loclog);
		//if (aType == -1) Log1(" Useless file !");
	};

/*
	void scanForMatch(string foldername, map<string, map<string, int>>& _result) {
		//map<string, map<string, int>> _result;
		map<string, map<string, int>> result_;
		string loclog = "\n";
		string fullpath = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)" + foldername + R"(\_1stPerson\)";
		
		//auto checkFile = [](aniMap* ani, string pathString, const string SfileName0, map<string, map<string, int>>& _result)  {
		//	if (_stricmp(pathString.c_str(), ".kf") == 0) {
		//		ani -> scanFiles(SfileName0, _result);
		//	}
		//};

		auto checkFile = [](aniMap* ani, const string SfileName0, map<string, map<string, int>>& _result) {

			ani->scanFiles(SfileName0, _result);

		};

		if (filesystem::exists(fullpath))
		{
			//Log1(" SCANNING " + fullpath);
			loclog += " SCANNING " + fullpath;
			//std::vector<std::thread> threadsScan;

			for (filesystem::directory_iterator iter(fullpath.c_str()), end; iter != end; ++iter)
			{
				const auto& path = iter->path();
				const auto& fileName = path.filename();
				const string SfileName0 = fileName.string();
				
				
				//vector<int> params;
				//Log1(" Checking file " + SfileName0);
				//checkFile(this, path.extension().string(), SfileName0, _result);

				//threadsScan.push_back(thread(checkFile, this, path.extension().string(), SfileName0, ref(_result)));
				//threadsScan.push_back(thread(checkAFile, path.extension().string(), SfileName0, ref(_result)));
				//std::promise<int> prom;
				if (_stricmp(path.extension().string().c_str(), ".kf") == 0) {
					//scanFiles(SfileName0, _result);
					//checkFile(this, SfileName0, _result);
					//threadsScan.push_back(thread(checkFile,this, SfileName0, ref(result_)));
					future<void> fut = async(checkFile, this, SfileName0, ref(_result));
					chrono::milliseconds span(100);
				}

			}

			//for (auto& thS : threadsScan) thS.join();
		}
		Log1(loclog);

		
		//return _result;
	}

	*/

	future<map<string, map<string, int>>> scanForMatch1(string foldername) {
		map<string, map<string, int>> _result;
		future<map<string, map<string, int>>> fut;
		
		string loclog = "\n";
		string fullpath = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)" + foldername + R"(\_1stPerson\)";

		auto checkFile = [](aniMap* ani, const string SfileName0) {

			map<string, map<string, int>> _result;

			ani->scanFiles(SfileName0, _result);

			return _result;
		};

		if (filesystem::exists(fullpath))
		{
			//Log1(" SCANNING " + fullpath);
			loclog += " SCANNING " + fullpath;
			//std::vector<std::thread> threadsScan;

			for (filesystem::directory_iterator iter(fullpath.c_str()), end; iter != end; ++iter)
			{
				const auto& path = iter->path();
				const auto& fileName = path.filename();
				const string SfileName0 = fileName.string();

				if (_stricmp(path.extension().string().c_str(), ".kf") == 0) {
					scanFiles(SfileName0, _result);
					fut = async(launch::async, checkFile, this, SfileName0);
					//fut.wait();
					chrono::milliseconds span(100);
				}

			}

		}
		Log1(loclog);


		return fut;
	};

	map<string, map<string, int>> scanForMatch2(string foldername) {
		map<string, map<string, int>> _result;
		future<map<string, map<string, int>>> fut;

		string loclog = "\n";
		string fullpath = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)" + foldername + R"(\_1stPerson\)";

		auto checkFile = [](aniMap* ani, const string SfileName0, map<string, map<string, int>>& _result) {

			ani->scanFiles(SfileName0, _result);

			return _result;
		};

		if (filesystem::exists(fullpath))
		{
			//Log1(" SCANNING " + fullpath);
			loclog += " SCANNING " + fullpath;
			//std::vector<std::thread> threadsScan;

			for (filesystem::directory_iterator iter(fullpath.c_str()), end; iter != end; ++iter)
			{
				const auto& path = iter->path();
				const auto& fileName = path.filename();
				const string SfileName0 = fileName.string();

				if (_stricmp(path.extension().string().c_str(), ".kf") == 0) {
					scanFiles(SfileName0, _result);
					fut = async(launch::async, checkFile, this, SfileName0, ref(_result));
					//fut.wait();
					chrono::milliseconds span(100);
				}

			}

		}
		Log1(loclog);


		return _result;
	};

};

struct subMap {
	string type;
	int typeData;
	vector<int> reloads;
	vector<int> attacks;
	vector<int> grips;

	string Sreloads;
	string Sattacks;
	string Sgrips;
};



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

struct folderMap {
	string folderName;
	map<string, map<string, int>> parsed;
	vector<vector<int>> typeParams;
	vector<subMap> subMaps;

	bool getParams(aniMap ani, string folder) {
		folderName = folder;
		

		//auto scan = [](aniMap ani, string folder, map<string, map<string, int>>& parsed) {
		//	ani.scanForMatch(folder, parsed);
		//};

		//ani.scanForMatch(folder, parsed);

		//future<map<string, map<string, int>>> parsedF = ani.scanForMatch1(folder);
		//parsed = parsedF.get();
		parsed = ani.scanForMatch2(folder);
		//future<void> fut = async(scan,ani, folder, ref(parsed));
		//chrono::milliseconds span(1000);

		for (auto data : parsed) {
			
			//int type = distance(ani.Type.begin(), find(ani.Type.begin(), ani.Type.end(), data.first));
			int type = data.first.back() - '0';
			//Log1(("Cheching from parsed: [" + to_string(type) + "] ").c_str());
			string loclog = "Cheching from parsed: [" + data.first + "] " + to_string(type) + "\n";
			//Log1(("Cheching from parsed: [" + data.first + "] " + to_string(type)).c_str());
			subMap currenSM;
			currenSM.type = data.first;
			currenSM.typeData = type;

			for (auto param : data.second) {

				//parameters.push_back(param.second);
				if (param.first.find("reload") != param.first.npos) {
					currenSM.reloads.push_back(param.second);
					currenSM.Sreloads += " " + to_string(param.second);
					//Log1(("Found reload " + to_string(param.second)  + " in [" + to_string(type) + "] ").c_str());
					loclog += "Found reload " + to_string(param.second) + " in [" + to_string(type) + "] \n";
				}
				else if (param.first.find("attack") != param.first.npos || param.first.find("place") != param.first.npos) {
					currenSM.attacks.push_back(param.second);
					currenSM.Sattacks += " " + to_string(param.second);
					//Log1(("Found attack " + to_string(param.second) + " in [" + to_string(type) + "] ").c_str());
					loclog += "Found attack " + to_string(param.second) + " in [" + to_string(type) + "] \n";
				}
				else currenSM.grips.push_back(param.second);
			}
			for (auto i : currenSM.reloads) {
				for (auto j : currenSM.attacks) {
					typeParams.push_back({ type, -1, i, j });
				}
			}
			subMaps.push_back(currenSM);
			Log1(loclog);
		}
	}

	map<string, vector<int>> getWeaponScan(vector<int> weaponData) {
		map<string, vector<int>> matchMap;
		for (auto param : typeParams) {
			//Log1(" TARGET param: [ " + to_string(weaponData[0]) + " " + to_string(weaponData[1]) + " " + to_string(weaponData[2]) + " " + to_string(weaponData[3]) + "] ");
			if (param[0] == weaponData[0] && param[2] == weaponData[2] && param[3] == weaponData[3]) {
				matchMap["matched"] = param;
				//Log1(" Matched param: [ " + to_string(param[0]) + " " + to_string(param[1]) + " " + to_string(param[2]) + " " + to_string(param[3]) + "] ");
			}
			else if (param[0] == weaponData[0] && param[2] == weaponData[2]) {
				matchMap["reload"].push_back(param[3]);
				//Log1(" Reload matched, attack: [ " + to_string(param[3]) + "] ");
			}
			else if (param[0] == weaponData[0] && param[3] == weaponData[3]) {
				matchMap["attack"].push_back(param[2]);
				//Log1(" Attack matched, reload: [ " + to_string(param[2])  + "] ");
			}
		}
		string attacks;
		string reloads;
		for (auto reload : matchMap["attack"]) {
			attacks += " " + to_string(reload);
		}
		for (auto reload : matchMap["reload"]) {
			reloads += " " + to_string(reload);
		}
		Log1(("Total half-matched reloads: [" + reloads + "] ").c_str());
		Log1(("Total half-matched attacks: [" + attacks + "] ").c_str());

		return matchMap;
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
bool writeType(folderMap _map);
bool writeType(vector<folderMap> _maps, string filename);
int writeTypesFolders(string namePart);
map<string, map<string, vector<int>>> DEEPscan(vector<int> weaponData);
bool writeWeapList(string filename, string modID = "FF");

//template <class num, class str>
//bool processTypesAndWrite3(string prefix, str modIDX, num eWeaponType, num handGrip, num reloadAnim, num attackAnim, num reversed, str filename) {
//	return processTypesAndWrite2(prefix, modIDX, eWeaponType, handGrip, reloadAnim, attackAnim, reversed, filename);
//}
// TBD: return of MAP{folder-params}
// TBD: return of LISTS{folders} per mod
// NOTE: keep reading jsons every time to keep things dynamic - it's not intended to be working in runtime.  
