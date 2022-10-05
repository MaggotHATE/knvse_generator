#pragma once
#include <filesystem>
#include "GameForms.h"
#include "json.h"

#include <stdexcept>
#include <thread>
#include <future>
#include<tuple>
//#include <chrono>


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

	string cleanName(string name) {
		string cleanedName = name;
		int reloadPos = name.find("reload");
		int attkPos = name.find("attack");
		if (name.find("_partial") == name.npos)	(reloadPos != name.npos ? cleanedName = name.substr(0,7) : (attkPos != name.npos ? cleanedName = name.substr(0,7) : cleanedName = name));

		return cleanedName;
	};

	string scanFiles (const string SfileName0, map<string, map<string, int>>& result) {
		string SfileName;
		transform(SfileName0.begin(), SfileName0.end(), std::back_inserter(SfileName), ::tolower);
		auto last = SfileName.length() - 6;
		string subname = SfileName.substr(3, last);
		string subname1 = subname.substr(0, last-2);
		//subname = cleanName(subname);
		//subname1 = cleanName(subname1);

		string loclog = "\n scanFiles "+ SfileName0 + ": " + subname + " or " + subname1 +" \n";
		//Log1(" Valid file !");
		int aType = -1;
		for (int i = 0; i < Type.size(); i++) {
			int pos = SfileName.find(Type[i]);
			//Log1(" Found " + Type[i] + " in " + to_string(pos));
			if (pos != string::npos && pos != -1) {

				aType = i;
				int aAttk = -1;
				try
				{
					//Log1(" Found " + to_string(k));
					if (Attk1.at(subname)) {

						aAttk = Attk1.at(subname);
						result[Type[i] + to_string(aType)][subname] = aAttk;

						loclog += "Found attack anim " + SfileName + " for " + to_string(aType) + " -1 -1 " + to_string(aAttk) + " \n";
					}
					
				}
				catch (const out_of_range& oor) {
					if (aAttk == -1)	try
					{
						//Log1(" Found " + to_string(k));
						if (Attk1.at(subname1)) {

							aAttk = Attk1.at(subname1);
							result[Type[i] + to_string(aType)][subname1] = aAttk;

							loclog += "Found additional attack anim " + SfileName + " for " + to_string(aType) + " -1 -1 " + to_string(aAttk) + " \n";
						}
						
					}
					catch (const out_of_range& oor) {
						//Log1("This weapontype wasn't defined: " + weapType[i]);
						//result[Type[i]]["noattacks"]= -2;
						for (auto attack : Attk1) {
							if (subname.find(attack.first) != subname.npos) {
								aAttk = attack.second;
								result[Type[i] + to_string(aType)][attack.first] = aAttk;

								loclog += "Found renamed attack anim " + SfileName + " for " + to_string(aType) + " -1 -1 " + to_string(aAttk) + " \n";
							}
						}
					}

					//Log1("This weapontype wasn't defined: " + weapType[i]);
					//result[Type[i]]["noattacks"]= -2;
				}
				
				if (aAttk != -1 && (Type[i] == "1hm" || Type[i] == "2hm" || Type[i] == "h2h") ) {
					result[Type[i] + to_string(aType)]["Grip"] = -1;
					result[Type[i] + to_string(aType)][Reload[0]] = -1;
					result[Type[i] + to_string(aType)][subname1] = 255;

					loclog += "Melees don't reload and use 255 for attack, skipping: " + SfileName + " for " + to_string(aType) + " -1 " + " -1 255 \n";
				} else if (aAttk == -1) {
					int aReload = -1;
					{
						for (int j = 0; j < Reload.size(); j++) {
							int pos1 = SfileName.find(Reload[j]);

							if (pos1 != string::npos && aReload == -1) {
								aReload = j;

								result[Type[i] + to_string(aType)]["Grip"] = -1;
								result[Type[i] + to_string(aType)][Reload[j]] = aReload;

								loclog += "Found reload anim " + SfileName + " for " + to_string(aType) + " -1 " + to_string(aReload) + " -1 \n";
							}
						}
					}
				}
				//if (aReload == -1) {
				//	
				//}
			}
		}
		//Log1(loclog);
		return loclog;
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

/*
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
					//chrono::milliseconds span(100);
				}

			}

		}
		Log1(loclog);


		return fut;
	};
	*/

	vector<map<string, map<string, int>>> scanForMatch2(string foldername) {
		map<string, map<string, int>> _result;
		vector<map<string, map<string, int>>> temps;
		vector<future<map<string, map<string, int>>>> tempsFuture1;

		//string loclog = "scanForMatch2 \n";
		string fullpath = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)" + foldername + R"(\_1stPerson\)";

		auto checkFile1 = [](aniMap* ani, const string SfileName0, string foldername) {

			map<string, map<string, int>> _result;

			string loclog1 = " \n";
			loclog1 += " SCANNING " + foldername + " \n" + ani->scanFiles(SfileName0, _result);
			Log1(loclog1);

			return _result;
		};

		if (filesystem::exists(fullpath))
		{
			//loclog += " SCANNING " + fullpath + " \n";

			for (filesystem::directory_iterator iter(fullpath.c_str()), end; iter != end; ++iter)
			{
				const auto& path = iter->path();
				const auto& fileName = path.filename();
				const string SfileName0 = fileName.string();

				if (_stricmp(path.extension().string().c_str(), ".kf") == 0) {
					tempsFuture1.push_back(async(launch::async, checkFile1, this, SfileName0, foldername));
				}

			}

		}

		for (auto& temp : tempsFuture1) {
			temps.push_back(temp.get());
		}

		//Log1(loclog);

		return temps;
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

struct typesBank {
	map <string, vector<string>> weapType;
	map <string, map<string, string>> weapCondi; // mod ID, <Folder name, Condition>
	map <string, map<string, string>> weapProps; // mod ID, <Folder name, Properties>
	map<string, vector<int>> typesMap;
	vector<string> typesMap1;
	vector<vector<int>> typesMap2;
	vector<string> typesMap3;
	vector<string> excluded;
	vector<string> options;

	bool processOrProp(TESObjectWEAP* weap, string input) {

		auto processAProp = [](TESObjectWEAP* weap, string input) {
			bool result = false;
			const char* str = input.c_str();
			hash<string> strH;
			float weapoProp = -999;
			int propPos;
			string resultStr = "";
			//string prop;
			//map<const char*, float> operVal;
			//operVal, prop = getStringNumAndOper(input);
			Log1("\n Result starts: " + to_string(result));
			//const char* str = prop.c_str();
			switch (*str) {
			case 'a':
				propPos = input.find("attackDmg");
				if (propPos != input.npos) {
					weapoProp = weap->attackDmg.GetDamage();
					Log1(" Found attackDmg of " + to_string(weapoProp));
					break;
				}

				else propPos = input.find("ammo");
				if (propPos != input.npos) {
					resultStr = weap->ammo.ammo->GetName();
					Log1(" Found ammo of " + resultStr);
					break;
				}

				else propPos = input.find("ammoUse");
				if (propPos != input.npos) {
					weapoProp = weap->ammoUse;
					Log1(" Found ammoUse of " + to_string(weapoProp));
					break;
				}

				else propPos = input.find("animShotsPerSec");
				if (propPos != input.npos) {
					weapoProp = weap->animShotsPerSec;
					Log1(" Found animShotsPerSec of " + to_string(weapoProp));
					break;
				}

				else propPos = input.find("animReloadTime");
				if (propPos != input.npos) {
					weapoProp = weap->animReloadTime;
					Log1(" Found animReloadTime of " + to_string(weapoProp));
					break;
				}
				else break;

			case 'c':
				propPos = input.find("clipRounds");
				if (propPos != input.npos) {
					weapoProp = weap->clipRounds.clipRounds;
					Log1(" Found clipRounds of " + to_string(weapoProp));
				}
				break;
			case 'f':
				propPos = input.find("fireRate");
				if (propPos != input.npos) {
					weapoProp = weap->fireRate;
					Log1(" Found fireRate of " + to_string(weapoProp));
				}
				break;
			case 'h':
				propPos = input.find("health");
				if (propPos != input.npos) {
					weapoProp = weap->health.health;
					Log1(" Found health of " + to_string(weapoProp));
					break;
				}
			case 'H':
				propPos = input.find("HasScope");
				if (propPos != input.npos) {
					weapoProp = weap->HasScope();
					Log1(" Found HasScope of " + to_string(weapoProp));
					break;
				}
				else propPos = input.find("HasItemModEffect");
				if (propPos != input.npos) {
					for (int i = 1; i < 3; i++) resultStr += ":" + to_string(weap->GetItemModEffect(i)) + ":";
					Log1(" Found HasItemModEffect of " + resultStr);
					break;
				}
				else break;
			case 'I':
				propPos = input.find("IsNonPlayable");
				if (propPos != input.npos) {
					weapoProp = weap->IsNonPlayable();
					Log1(" Found IsNonPlayable of " + to_string(weapoProp));
				}
				break;
			case 'n':
				propPos = input.find("numProjectiles");
				if (propPos != input.npos) {
					weapoProp = weap->numProjectiles;
					Log1(" Found numProjectiles of " + to_string(weapoProp));
				}
				break;
			case 's':
				propPos = input.find("skillRequirement");
				if (propPos != input.npos) {
					weapoProp = weap->skillRequirement;
					Log1(" Found skillRequirement of " + to_string(weapoProp));
					break;
				}


				else propPos = input.find("strRequired");
				if (propPos != input.npos) {
					weapoProp = weap->strRequired;
					Log1(" Found strRequired of " + to_string(weapoProp));
					break;
				}

				else propPos = input.find("sightUsage");
				if (propPos != input.npos) {
					weapoProp = weap->sightUsage;
					Log1(" Found sightUsage of " + to_string(weapoProp));
					break;
				}

				else propPos = input.find("soundLevel");
				if (propPos != input.npos) {
					weapoProp = weap->soundLevel;
					Log1(" Found soundLevel of " + to_string(weapoProp));
					break;
				}
				else break;
			case 'v':
				propPos = input.find("value");
				if (propPos != input.npos) {
					weapoProp = weap->value.value;
					Log1(" Found value of " + to_string(weapoProp));
				}
				break;
			case 'w':
				propPos = input.find("weight");
				if (propPos != input.npos) {
					weapoProp = weap->weight.weight;
					Log1(" Found weight of " + to_string(weapoProp));
					break;
				}

				else propPos = input.find("weaponSkill");
				if (propPos != input.npos) {
					weapoProp = weap->weaponSkill;
					Log1(" Found weaponSkill of " + to_string(weapoProp));
				}
			}

			if (weapoProp != -999 || resultStr != "") {
				//Log1("\n Found a weapon input of " + to_string(weapoProp));
				float propVal;
				string operAndNum = input.substr(propPos);
				Log1(" Found a input operand with a number: " + operAndNum);
				int eqPos = operAndNum.find("=");
				int notEqPos = operAndNum.find("!");
				int morePos = operAndNum.find(">");
				int lessPos = operAndNum.find("<");
				if (eqPos != operAndNum.npos) {

					string num = operAndNum.substr(eqPos + 1);
					if (resultStr != "") {
						Log1(" Found a input name with =: " + num);
						if (resultStr.find(":") != resultStr.npos) result = (notEqPos == operAndNum.npos) ? (resultStr.find(":" + num + ":") != resultStr.npos) : (resultStr.find(":" + num + ":") == resultStr.npos);
						else result = (notEqPos == operAndNum.npos) ? (resultStr.find(num) != resultStr.npos) : (resultStr.find(num) == resultStr.npos);
					}
					else {
						Log1(" Found a input number with =: " + num + " with a " + ((lessPos == operAndNum.npos) ? ">" : "<"));
						propVal = stoi(num);
						Log1(" Found a value: " + to_string(propVal));
						if (lessPos == operAndNum.npos && morePos == operAndNum.npos) {
							result = (notEqPos == operAndNum.npos) ? propVal == weapoProp : propVal != weapoProp;
						}
						else if (lessPos != operAndNum.npos) {
							result = (weapoProp <= propVal);
						}
						else {
							result = (weapoProp >= propVal);
						}
					}

				}
				else if (lessPos != operAndNum.npos && resultStr == "") {
					string num = operAndNum.substr(lessPos + 1);
					Log1(" Found a input number with <: " + num);
					propVal = stoi(num);

					result = weapoProp < propVal;

				}
				else {
					string num = operAndNum.substr(morePos + 1);
					Log1(" Found a input number with >: " + num);
					propVal = stoi(num);

					result = weapoProp > propVal;
				}
			}
			Log1(" Result is: " + to_string(result) + " \n");
			return result;
		};
		//// Lambda end

		int orPos = input.find("||");
		if (orPos != input.npos) {
			Log1("\n Found a divider: || in " + input);
			return (processOrProp(weap, input.substr(0, orPos)) || processOrProp(weap, input.substr(orPos + 2)));
		}
		else return processAProp(weap, input);
	}

	bool getWeaponProp(TESObjectWEAP* weap, string property) {

		string propCheck = property;
		int divPos = propCheck.find("&&");
		bool result = processOrProp(weap, propCheck.substr(0, divPos));
		while (divPos != propCheck.npos) {
			propCheck = propCheck.substr(divPos + 2);
			Log1("\n Found a divider: && " + propCheck);
			divPos = propCheck.find("&&");
			string subProp = propCheck.substr(0, divPos);
			result = (result && processOrProp(weap, subProp));
		}

		return result;
	}

	bool hasProperty(string modID, TESObjectWEAP* weap, string matchFolder) {
		bool tmp = weapProps[modID].find(matchFolder) != weapProps[modID].end();
		if (tmp == false) return weapProps["FF"].find(matchFolder) != weapProps["FF"].end();
		else return tmp;
	}

	bool hasTrueProperty(string modID, TESObjectWEAP* weap, string matchFolder) {
		bool preTmp = hasProperty(modID, weap, matchFolder);

		if (preTmp == true) {
			bool tmp = preTmp && getWeaponProp(weap, weapProps[modID][matchFolder]) == true;
			if (tmp == false) return preTmp && getWeaponProp(weap, weapProps["FF"][matchFolder]) == true;
			else return tmp;
		}
		else return false;
	}


	vector<bool> hasCondition(string modID, string matchFolder) {
		Log1(" Checking first time " + matchFolder);
		bool firstCondiCheck = getCondiElement(modID, matchFolder) != "";
		bool hasLocal = true;
		Log1(" : condition " + to_string(firstCondiCheck));
		//if (hasProrerty == false ) bool hasProrerty = definedType.weapProps["FF"].find(matchFolder) == definedType.weapProps["FF"].end() || getWeaponProp(weap, definedType.weapProps["FF"][matchFolder]) == true;
		if (firstCondiCheck == false) {
			firstCondiCheck = getCondiElement("FF", matchFolder) != "";
			hasLocal = false;
			Log1(" Checking Second time (" + to_string(weapCondi["FF"].size()) + ") " + matchFolder + " : condition " + to_string(firstCondiCheck));
		}
		return { firstCondiCheck, hasLocal };
	}

	bool folderInMod(string fileModName, string type) {
		if (!weapType[fileModName].empty()) {
			return (find(weapType[fileModName].begin(), weapType[fileModName].end(), type) != weapType[fileModName].end());
		}
		else if (!weapType["FF"].empty()) {
			return (find(weapType["FF"].begin(), weapType["FF"].end(), type) != weapType["FF"].end());
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

	string getCondiElement(string modID, string folder) {
		//Log1(" Trying getCondiElement for " + modID + " and " + folder);
		try {
			if (!weapCondi.at(modID).empty()) {
				//Log1(" Some conditions for mod " + modID);
				try {
					if (!weapCondi.at(modID).at(folder).empty()) {
						//Log1(" Some conditions for folder " + folder);
						return weapCondi.at(modID).at(folder);
					}
					else return "";
				}
				catch (const out_of_range& oor) {
					return "";
				}
			}
		}
		catch (const out_of_range& oor) {
			return "";
		}
		return "";
	}
};

struct weaponsBank {
	map<int, map<string, string>> formAndFolder; // mod ID, <weapon hexed, folder>
	map<int, map<string, vector<string>>> folderAndForm; // mod ID, <weapon hexed, folder>
	map<int, map<string, vector<string>>> formAndFoldersWCondi; // mod id, <weapon hexed, <folder, condition>>

	map<string, string> folderAndCondi; // <folder, condition>

	map<string, vector<int>> unassigned;

	bool pushCondi(typesBank& definedType, int IDX, string hexedID, string modID, string folder) {
		if (hexedID != "" && folder != "" && IDX >= 0) {
			formAndFoldersWCondi[IDX][hexedID].push_back(definedType.weapCondi[modID][folder]);
			return true;
		}
		else return false;
	}

	bool pushFolderAndForm(int IDX, string folder, string hexedID) {
		if (hexedID != "" && folder != "" && IDX >= 0) {
			folderAndForm[IDX][folder].push_back(hexedID);
			return true;
		}
		else return false;
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

	string log1;

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
			for (auto type : definedTypes.weapType["FF"]) {
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
		vector<string>& weaptype = (!definedTypes.weapType[fileModName].empty()) ? definedTypes.weapType[fileModName] : definedTypes.weapType["FF"];

		for ( auto type : weaptype) {
			//Log1(" Checking type " + type);
			for (int i = 0; i < definedTypes.typesMap1.size(); i++) {
				if (definedTypes.typesMap1[i] == type && isThisType(definedTypes.typesMap2[i])) {
						matching.push_back(type);
				}
			}
		}
		if (matching.size() > 0) {
			Log1("\n Weapon data for " + name + "(" + hexedID + ")" + " : mod ID " + FormatString(R"(%X)", modIDX) + " = " + fileModName);

			Log1(" Animation type: " + to_string(typeParams[0]));
			Log1(" Grip type: " + to_string(typeParams[1]));
			Log1(" Reload animation: " + to_string(typeParams[2]));
			Log1(" Attack animation: " + to_string(typeParams[3]) + " \n");
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
		vector<string>& weaptype = (!definedTypes.weapType[fileModName].empty()) ? definedTypes.weapType[fileModName] : definedTypes.weapType["FF"];

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
	
	bool aParamCheck(int scanned, int real) {
		if (scanned == real || scanned == -1) return true; else return false;
	}

	bool getParams(aniMap ani, string folder) {
		const auto then = chrono::system_clock::now();
		folderName = folder;
		

		//auto scan = [](aniMap ani, string folder, map<string, map<string, int>>& parsed) {
		//	ani.scanForMatch(folder, parsed);
		//};

		//ani.scanForMatch(folder, parsed);

		//future<map<string, map<string, int>>> parsedF = ani.scanForMatch1(folder);
		//parsed = parsedF.get();
		//parsed = ani.scanForMatch2(folder);
		vector<map<string, map<string, int>>> temps = ani.scanForMatch2(folder);
		//vector<map<string, map<string, int>>> temps;// = ani.scanForMatch2(folder);
		//vector<future<map<string, map<string, int>>>> tempsF = ani.scanForMatch3(folder);
		string loclog = " CHECKING " + folder + " \n";

		for (auto temp : temps) {
			for (auto tem : temp) {
				for (auto te : tem.second) {
					//loclog += " Reassembling " + tem.first + ": Item " + te.first + " = " + to_string(te.second) + " \n";
					parsed[tem.first][te.first] = te.second;
				}
			}
		}
		//future<void> fut = async(scan,ani, folder, ref(parsed));
		//chrono::milliseconds span(1000);

		for (auto data : parsed) {
			
			//int type = distance(ani.Type.begin(), find(ani.Type.begin(), ani.Type.end(), data.first));
			int type = data.first.back() - '0';
			//Log1(("Cheching from parsed: [" + to_string(type) + "] ").c_str());
			loclog += "Checking from parsed: [" + data.first + "] " + to_string(type) + "\n";
			//Log1(("Cheching from parsed: [" + data.first + "] " + to_string(type)).c_str());
			subMap currenSM;
			currenSM.type = data.first;
			currenSM.typeData = type;

			for (auto param : data.second) {

				loclog += " Reading: [" + param.first + "] \n";
				
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
			//Log1(loclog);
		}

		const auto now = chrono::system_clock::now();
		const auto diff = chrono::duration_cast<chrono::milliseconds>(now - then);
		Log1(FormatString("Params got in %d ms", diff.count()));
	}

	map<string, vector<int>> getWeaponScan(vector<int> weaponData) {
		const auto then = chrono::system_clock::now();
		map<string, vector<int>> matchMap;
		string loclog;
		for (auto param : typeParams) {
			//Log1(" TARGET param: [ " + to_string(weaponData[0]) + " " + to_string(weaponData[1]) + " " + to_string(weaponData[2]) + " " + to_string(weaponData[3]) + "] ");
			if (aParamCheck(param[0], weaponData[0]) && aParamCheck(param[2], weaponData[2]) && aParamCheck(param[3], weaponData[3])) {
				matchMap["matched"] = param;
				//Log1(" Matched param: [ " + to_string(param[0]) + " " + to_string(param[1]) + " " + to_string(param[2]) + " " + to_string(param[3]) + "] ");
			}
			else if (aParamCheck(param[0], weaponData[0]) && aParamCheck(param[2], weaponData[2])) {
				matchMap["reload"].push_back(param[3]);
				//Log1(" Reload matched, attack: [ " + to_string(param[3]) + "] ");
			}
			else if (aParamCheck(param[0], weaponData[0]) && aParamCheck(param[3], weaponData[3])) {
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
		loclog +=("Total half-matched reloads: [" + reloads + "] ");
		loclog += ("Total half-matched attacks: [" + attacks + "] ");
		//Log1(loclog);
		const auto now = chrono::system_clock::now();
		const auto diff = chrono::duration_cast<chrono::milliseconds>(now - then);
		Log1(FormatString("Weapon scanned in %d ms", diff.count()));

		return matchMap;
	}
};

//weaponsBank processTypesMod1(vector<string>& weapType, map<string, vector<int>>& typesMap, vector<string> excluded, string modID = "FF");
//bool processTypesAndWrite(string prefix, string modIDX = "FF");
bool processTypesAndWrite2(string prefix, string modIDX = "FF", int eWeaponType = 0,  int handGrip = 0, int reloadAnim = 0, int attackAnim = 0, int reversed = 0, string typesfilename = "_", string weapsfilename = "_");

typesBank readTypes(vector<int> params, string typesfilename = "_", string weapsfilename = "_");
weaponsBank processTypesMod2(typesBank& definedType, string modID);
bool readAutoTypes(string typesfilename);
weaponType getWeaponData1(TESObjectWEAP* weap);
vector<string> matchAWeapon(TESObjectWEAP* weap);
vector<string> matchAWeapon1(weaponType& weaps, vector<int> params = {});
nlohmann::json writeAweapon(string prefix, TESObjectWEAP* weap, int reversed = 0, string folder = "_");
nlohmann::json writeAweapon2(string prefix, TESObjectWEAP* weap, int reversed = 0, string folder = "_");
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
