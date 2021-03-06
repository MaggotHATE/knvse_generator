#include "GameForms.h"
#include "GameData.h"
#include "GameAPI.h"
#include "GameObjects.h"
#include "GameRTTI.h"
#include <fstream>
#include <utility>
#include <regex>
#include "file_addon.h"
#include "json.h"
#include <time.h>


using namespace std;
using namespace nlohmann;
vector<string> EXCLUDED_NAMES = { "empty" , "lastshot" };
//string EXCLUDE = "FalloutNV.esm, DeadMoney.esm, HonestHearts.esm, OldWorldBlues.esm, LonesomeRoad.esm, GunRunnersArsenal.esm";

//
//Json::Value funcGetJson(char*& filename)
//{
//    std::fstream fs;
//    Json::Value root;
//    fs.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
//    std::locale loc = std::locale("Russian");
//    fs.imbue(loc);
//    Json::CharReaderBuilder rbuilder;
//    rbuilder["collectComments"] = false;
//    std::string errs;
//    Json::parseFromStream(rbuilder, fs, &root, &errs);
//
//    fs.close();
//    return root;
//}
//
//void jsonWriteFile(char* filename, bool isUTF8 = true)
//{
//	Json::Value root;
//	std::fstream fs;
//	fs.open(filename, std::ios::binary | std::fstream::in | std::ofstream::out | std::ofstream::trunc);
//
//	//std::locale loc = std::locale("Russian_Russia.1251");
//	//std::locale loc = std::locale("Russian");
//	//fs.imbue(loc);
//	Json::StreamWriterBuilder builder;
//	if (isUTF8 == true) {
//		builder["emitUTF8"] = true;
//	}
//	else {
//		builder["emitUTF8"] = false;
//	}
//	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
//	writer->write(root, &fs);
//}
//
//void jsonRewriteFile(Json::Value root, string filename, bool isUTF8 = true)
//{
//	std::fstream fs;
//	fs.open(filename, std::ios::binary | std::fstream::in | std::ofstream::out | std::ofstream::trunc);
//
//	//std::locale loc = std::locale("Russian_Russia.1251");
//	std::locale loc = std::locale("Russian");
//	fs.imbue(loc);
//	Json::StreamWriterBuilder builder;
//	if (isUTF8 == true) {
//		builder["emitUTF8"] = true;
//	}
//	else {
//		builder["emitUTF8"] = false;
//	}
//	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
//	writer->write(root, &fs);
//}

bool checkName(string name, vector<string> excludes) {
	string lowName;
	transform(name.begin(), name.end(), std::back_inserter(lowName), ::tolower);
	for (auto exclude : excludes) {
		if (lowName.find(exclude) != lowName.npos) return false;
	}

	return true;
}

int findString(string base, string search) {
	auto pos = base.find(search);
	if (pos != base.npos) return pos; else return 999;
}

const auto GetHasType2 (vector<int>& weapParams, vector<int>& typesData) // -1 is "any" for better flexibility
{
	if (
		(weapParams[0] == typesData[0] || typesData[0] == -1) &&
		(weapParams[1] == typesData[1] || typesData[1] == -1) &&
		(weapParams[2] == typesData[2] || typesData[2] == -1) &&
		(weapParams[3] == typesData[3] || typesData[3] == -1)
		) {

		return true;
	}
	return false;
}

void Log1(const string& msg)
{
	_MESSAGE("%s", msg.c_str());
}

string GetCurPath()
{
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}

int HexStringToInt(const string& str)
{
	char* p;
	const auto id = strtoul(str.c_str(), &p, 16);
	if (*p == 0)
		return id;
	return -1;
}

TESObjectWEAP* Actor::GetWeaponForm() const
{
	auto* weaponInfo = this->baseProcess->GetWeaponInfo();
	if (!weaponInfo)
		return nullptr;
	return weaponInfo->weapon;
}

typesBank readTypes(vector<int> params, string typesfilename, string weapsfilename) {
	if (typesfilename != "_" && typesfilename.find(".json") == typesfilename.npos) typesfilename += ".json";
	if (weapsfilename != "_"  && weapsfilename.find(".json") == weapsfilename.npos) weapsfilename += ".json";
	auto hasIt = [](string input, string check) {
		int posIt = input.find(check);
		if (posIt != input.npos) return posIt; else return -1;
	};
	auto hasItAll = [](string input, string check) {
		int posIt = input.find(check);
		string result = "";
		if (posIt != input.npos) {
			int openBracket = input.find("[");
			int closeBracket = input.find("]");
			result = input.substr(openBracket + 1, closeBracket - openBracket - 1);
		}

		return result;
	};
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types)";
	typesBank typesDB;
	if (filesystem::exists(dir))
	{
		for (filesystem::directory_iterator iter(dir.c_str()), end; iter != end; ++iter)
		{
			const auto& path = iter->path();
			const auto& fileName = path.filename();
			const auto& SfileName = fileName.string();
			if (_stricmp(path.extension().string().c_str(), ".json") == 0 && (SfileName.find("autogen")!= 0))
			{
				Log1("\nReading Types from JSON file " + SfileName);
				if (!params.empty()) Log1("\nParams given " + to_string(params[0]));
				Log1("\nFiles: " + typesfilename + " and " + weapsfilename);
				try
				{
					ifstream i(iter->path());
					json j;
					i >> j;
					if (j.is_array())
					{
						for (auto& elem : j)
						{
							if (!elem.is_object())
							{
								Log1("JSON error: expected object with mod, form and folder fields");
								continue;
							}
							auto typeData = elem.contains("typedata") ? &elem["typedata"] : nullptr;
							const auto& types = elem.contains("weapontypes") ? &elem["weapontypes"] : nullptr;
							const auto& folderType = elem.contains("folderType") ? elem["folderType"].get<string>() : "";
							const auto& modExclude = elem.contains("exclude") ? &elem["exclude"] : nullptr;
							const auto& options = elem.contains("options") ? &elem["options"] : nullptr;
							const auto& mod = elem.contains("mod") ? elem["mod"].get<string>() : "";
							//const auto& condition = elem.contains("condition") ? elem["condition"].get<string>() : "";
							//Log1("Defining types");
							if (typeData && folderType != "" && (typesfilename == "_" || SfileName == typesfilename)) { // defining types
								if (!typeData->is_array())
								{
									continue;
								}
								else {
									vector<int> tempData;
									//ranges::transform(*typeData, back_inserter(typesDB.typesMap[folderType]), [&](auto& i) {return i; });
									ranges::transform(*typeData, back_inserter(tempData), [&](auto& i) {return i; });
									if (params.empty() || GetHasType2(params, tempData) == true) {
										//typesDB.typesMap[folderType] = tempData;
										typesDB.typesMap1.push_back(folderType);
										typesDB.typesMap2.push_back(tempData);
										typesDB.typesMap3.push_back(SfileName);
										//Log1("Filename = " + SfileName);
									}

									//vector<int> tempData;
									//ranges::transform(*typeData, back_inserter(tempData), [&](auto& i) {return i; });
									//typesDB.typesMap1[tempData] = folderType;
									//for (auto& iter: typesDB.typesMap) {
									//	Log1("Defining types " + iter.first);
									//}
								}
							}

							if (types && (weapsfilename == "_" || SfileName == weapsfilename)) { // reading applied types

								if (!types->is_array())
								{									
									continue;
								}
								else {
									string modChecked = mod;
									if (modChecked == "") {
										//ranges::transform(*types, back_inserter(typesDB.weapType["GLOBAL"]), [&](auto& i) {return i.template get<string>(); });
										//Log1("reading applied types for GLOBAL: " + typesDB.weapType["GLOBAL"].back());
										modChecked = "FF";
									}
									//else {
									string logloc = "Reading inside for " + modChecked + ": \n";
									ranges::transform(*types, back_inserter(typesDB.weapType[modChecked]), [&](auto& i) {
										string iString = i.template get<string>();
										int condiPos = findString(iString, "+condition");
										int propPos = findString(iString, "+properties");

										if (condiPos == 999 && propPos == 999) {
											logloc += ("\n Pure folder name: "+ iString);
											return iString;
										}
										else {
											logloc += ("\n Conditioned folder name: " + iString);
											string iStringName;
											string iStringData;
											string iStringDataCondi;
											string iStringDataProp;
											
											//logloc += ("\n Conditioned/propertied folder name: " + iString);
											int firstPlus = iString.find("+");
											iStringName = iString.substr(0, firstPlus);
											iStringData = iString.substr(firstPlus);
											Log1("Separated into " + iStringName + " and " + iStringData + " \n");
											int firstBracket = iStringData.find("[");
											int lastBracket = iStringData.rfind("[");
											int firstRBracket = iStringData.find("]");
											int lastRBracket = iStringData.rfind("]");

											Log1("Brackets found: " + to_string(firstBracket) + " , " + to_string(firstRBracket) + " , " + to_string(lastBracket) + " , " + to_string(lastRBracket) + " \n");

											if (firstBracket != iStringData.npos && firstRBracket != iStringData.npos) {
												string iStringDataFirst = iStringData.substr(0, firstRBracket + 1);
												string iStringDataSecond = iStringData.substr(firstRBracket + 1);
												Log1("Separated data into " + iStringDataFirst + " and " + iStringDataSecond + " \n");
												iStringDataCondi = hasItAll(iStringDataFirst, "condition");
												if (iStringDataCondi == "") iStringDataCondi = hasItAll(iStringDataSecond, "condition");
												// parsing properties
												iStringDataProp = hasItAll(iStringDataFirst, "properties");
												if (iStringDataProp == "") iStringDataProp = hasItAll(iStringDataSecond, "properties");
											}


											if (iStringDataCondi != "") {
												logloc += ("\n Conditions: [" + modChecked + "][" + iStringName + "] = " + iStringDataCondi);
												//logloc +=("\n Properties: " + iStringDataProp);
												typesDB.weapCondi[modChecked][iStringName] = iStringDataCondi;
												//typesDB.weapProps[modChecked][iStringName] = iStringDataProp;
												//if (condiPos != 999);
												//if (propPos != 999);
											}
											if (iStringDataProp != "") {
												logloc +=("\n Properties: " + iStringName + " with " + iStringDataProp);
												typesDB.weapProps[modChecked][iStringName] = iStringDataProp;
											}


											return iStringName;
										}
											
										} );
									Log1("reading applied types for " + modChecked + ": " + typesDB.weapType[modChecked].back());
									Log1(logloc);
									//}
								}
							}

							if (modExclude) {
								if (!modExclude->is_array())
								{
									continue;
								}
								else {
									ranges::transform(*modExclude, back_inserter(typesDB.excluded), [&](auto& i) {return i.template get<string>(); });
									Log1("reading exclusions: " + typesDB.excluded.back());
								}
							}

							if (options) {
								if (!options->is_array())
								{
									continue;
								}
								else {
									ranges::transform(*options, back_inserter(typesDB.options), [&](auto& i) {return i.template get<string>(); });
									Log1("reading options: " + typesDB.options.back());
								}
							}

							//Log1("processTypesMod1 check");
							//if (!typesDB.typesMap.empty() && !typesDB.weapType.empty()) {
							//	weaponsBank weaps = processTypesMod1(-1, typesDB.weapType, typesDB.typesMap);
							//	writeJson(weaps);
							//}
						}
					}
					else
						Log1(iter->path().string() + " does not start as a JSON array");
				}
				catch (json::exception& e)
				{
					Log1("The JSON is incorrectly formatted! It will not be applied.");
					Log1(FormatString("JSON error: %s\n", e.what()));
				}
			}
		}
	}
	else
	{
		Log1(dir + " does not exist.");
	}
	return typesDB;
}

bool readAutoTypes(string typesfilename) {
	if (typesfilename.find(".json") == typesfilename.npos) typesfilename += ".json";
	auto hasIt = [](string input, string check) {
		int posIt = input.find(check);
		if (posIt != input.npos) return posIt; else return -1;
	};
	auto hasItAll = [](string input, string check) {
		int posIt = input.find(check);
		string result = "";
		if (posIt != input.npos) {
			int openBracket = input.find("[");
			int closeBracket = input.find("]");
			result = input.substr(openBracket + 1, closeBracket - openBracket - 1);
		}

		return result;
	};
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types)";
	string execPrefix;
	string execID;
	string execTypes;
	string execWeaps;
	vector<int> execParams;
	int execReversed = 0;
	if (filesystem::exists(dir))
	{
		for (filesystem::directory_iterator iter(dir.c_str()), end; iter != end; ++iter)
		{
			const auto& path = iter->path();
			const auto& fileName = path.filename();
			const auto& SfileName = fileName.string();
			if (_stricmp(path.extension().string().c_str(), ".json") == 0 && (SfileName.find("autogen") == 0) && (SfileName == typesfilename))
			{
				Log1("\n Reading Autogen from JSON file " + SfileName);
				Log1("\n Autogen file: " + typesfilename);
				try
				{
					ifstream i(iter->path());
					json j;
					i >> j;
					if (j.is_array())
					{
						for (auto& elem : j)
						{
							if (!elem.is_object())
							{
								Log1("JSON error: expected object with mod, form and folder fields");
								continue;
							}
							
							const auto& prefix = elem.contains("prefix") ? elem["prefix"].get<string>() : "";
							const auto& modID = elem.contains("modID") ? elem["modID"].get<string>() : "";
							const auto& typesFile = elem.contains("typesFile") ? elem["typesFile"].get<string>() : "";
							const auto& weaponsFile = elem.contains("weaponsFile") ? elem["weaponsFile"].get<string>() : "";
							auto typeData = elem.contains("typedata") ? &elem["typedata"] : nullptr;
							const auto& reversed = elem.contains("reversed") ? elem["reversed"].get<string>() : "";
							//const auto& condition = elem.contains("condition") ? elem["condition"].get<string>() : "";
							//Log1("Defining types");
							if (modID != "" && prefix != "" ) { // defining types
								Log1("Reading: modID and prefix");
								execPrefix = prefix;
								execID = modID;
								if (typesFile == "") execTypes = "_";
								if (weaponsFile == "") execWeaps = "_";
								if (reversed != "") execReversed = stoi(reversed);
								if (typeData){
									if (!typeData->is_array())
									{
										continue;
									}
									else {
									
										//execParams[0] = typeData[0];
										//execParams[1] = typeData[1];
										//execParams[2] = typeData[2];
										//execParams[3] = typeData[3];
										ranges::transform(*typeData, back_inserter(execParams), [&](auto& i) {return i; });
										//vector<int> tempData;
										//ranges::transform(*typeData, back_inserter(tempData), [&](auto& i) {return i; });
										//typesDB.typesMap1[tempData] = folderType;
										//for (auto& iter: typesDB.typesMap) {
										//	Log1("Defining types " + iter.first);
										//}
									}
								}
								processTypesAndWrite2(execPrefix, execID, execParams[0], execParams[1], execParams[2], execParams[3], execReversed, typesFile, weaponsFile);
							}
						}
					}
					else
						Log1(iter->path().string() + " does not start as a JSON array");
				}
				catch (json::exception& e)
				{
					Log1("The JSON is incorrectly formatted! It will not be applied.");
					Log1(FormatString("JSON error: %s\n", e.what()));
				}
			}
		}
	}
	else
	{
		Log1(dir + " does not exist.");
	}
	return true;
}

bool thatWeapon(TESObjectWEAP* weap, vector<int>& params){
	if (params.empty() || (weap->eWeaponType == params[0] &&
							weap->handGrip == params[1] &&
							weap->reloadAnim == params[2] &&
							weap->attackAnim == params[3])
		) {
		return true;
	}
	return false;
}

/*
weaponType getWeaponData(TESForm* form, typesBank& definedType) {
	weaponType getweapon;

	auto weap = static_cast<TESObjectWEAP*>(form);
	getweapon.ID = weap->refID;
	getweapon.modIDX = weap->GetModIndex();
	getweapon.modIDXshort = getweapon.modIDX + (getweapon.modIDX * 0x00FFFFFF);
	getweapon.fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(getweapon.modIDX));
	getweapon.hexedID = FormatString(R"(%X)", getweapon.ID - getweapon.modIDXshort);

	string test = weap->GetName();
	Log1("Type to " + test + "(" + getweapon.hexedID + ")" + " : mod ID " + FormatString(R"(%X)", getweapon.modIDX) + " = " + getweapon.fileModName);

	getweapon.typeParams.push_back(weap->eWeaponType); //Animation type
	getweapon.typeParams.push_back(weap->handGrip); //Grip type
	getweapon.typeParams.push_back(weap->reloadAnim); //Reload animation (non-modded)
	getweapon.typeParams.push_back(weap->attackAnim); //Attack animation
	Log1("Animation type: " + to_string(getweapon.typeParams[0]));
	Log1("Grip type: " + to_string(getweapon.typeParams[1]));
	Log1("Reload animation: " + to_string(getweapon.typeParams[2]));
	Log1("Attack animation: " + to_string(getweapon.typeParams[3]));
	vector<string> matchFolders = getweapon.checkFolders2(definedType);
	if (matchFolders.size() == 1) {
		getweapon.typeFolder = matchFolders[0];
	}
	else if (matchFolders.size() > 1) {
		srand(time(NULL));
		getweapon.typeFolder = matchFolders[rand() % matchFolders.size()];
	}
	if (getweapon.typeFolder != "") Log1("Replacing animations with " + getweapon.typeFolder + " for " + test + "; refID " + getweapon.hexedID + "; mod ID " + FormatString(R"(%X)", getweapon.modIDXshort));
	
	return getweapon;
}
*/

weaponType getWeaponData1(TESObjectWEAP* weap) {
	weaponType getweapon;

	getweapon.ID = weap->refID;
	getweapon.modIDX = weap->GetModIndex();
	getweapon.modIDXshort = getweapon.modIDX + (getweapon.modIDX * 0x00FFFFFF);
	getweapon.fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(getweapon.modIDX));
	getweapon.hexedID = FormatString(R"(%X)", getweapon.ID - getweapon.modIDXshort);

	getweapon.name = weap->GetName();
	//Log1("Weapon data for " + getweapon.name + "(" + getweapon.hexedID + ")" + " : mod ID " + FormatString(R"(%X)", getweapon.modIDX) + " = " + getweapon.fileModName);

	getweapon.typeParams.push_back(weap->eWeaponType); //Animation type
	getweapon.typeParams.push_back(weap->handGrip); //Grip type
	getweapon.typeParams.push_back(weap->reloadAnim); //Reload animation (non-modded)
	getweapon.typeParams.push_back(weap->attackAnim); //Attack animation
	//Log1("Animation type: " + to_string(getweapon.typeParams[0]));
	//Log1("Grip type: " + to_string(getweapon.typeParams[1]));
	//Log1("Reload animation: " + to_string(getweapon.typeParams[2]));
	//Log1("Attack animation: " + to_string(getweapon.typeParams[3]));

	getweapon.log1 += "\n";
	getweapon.log1 += (" Animation type: " + to_string(getweapon.typeParams[0]));
	getweapon.log1 += (" Grip type: " + to_string(getweapon.typeParams[1]));
	getweapon.log1 += (" Reload animation: " + to_string(getweapon.typeParams[2]));
	getweapon.log1 += (" Attack animation: " + to_string(getweapon.typeParams[3]));
	getweapon.log1 += "\n";


	return getweapon;
}

/// outtake
	//auto getStringNumAndOper = [](string input) {
	//	string prop;
	//	const char* oper;
	//	map<const char*, float> result;
	//	int eqPos = input.find("=");
	//	int morePos = input.find(">");
	//	int lessPos = input.find("<");
	//	if (eqPos != input.npos) {
	//		if (morePos == input.npos && lessPos == input.npos) {
	//			result["="] = stoi(input.substr(eqPos + 1));
	//		}
	//		else result[(morePos != input.npos) ? ">=" : "<="] = stoi(input.substr(eqPos + 1));
	//	}
	//	else {
	//		result[(morePos != input.npos) ? ">" : "<"] = (morePos != input.npos) ? stoi(input.substr(morePos + 1)) : stoi(input.substr(lessPos + 1));
	//	}

	//	return result, prop;
	//};
///

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
		result = ( result && processOrProp(weap, subProp) );
	}

	return result;
}

weaponsBank processTypesMod2(typesBank& definedType, string modID)
{
	weaponsBank weapBank;

	NiTPointerMap<TESForm>* formsMap = *(NiTPointerMap<TESForm>**)0x11C54C0;
	int modIDX = HexStringToInt(modID);
	srand(time(NULL));

	for (auto mIter = formsMap->Begin(); mIter; ++mIter) { // credits to Yvileapsis for the iteration example

		auto checkAfolder = [](typesBank definedType, string modID, TESObjectWEAP* weap, string matchFolder) {
			bool hasProrerty = definedType.weapProps[modID].find(matchFolder) == definedType.weapProps[modID].end() || getWeaponProp(weap, definedType.weapProps[modID][matchFolder]) == true;
			//bool hasCondition = definedType.weapCondi[modID].find(matchFolder) != definedType.weapCondi[modID].end();
			bool hasCondition = definedType.getCondiElement(modID, matchFolder) != "";
			bool hasLocal = true;
			Log1(" Checking first time " + matchFolder + " : property " + to_string(hasProrerty) + "; condition " + to_string(hasCondition));
			//if (hasProrerty == false ) bool hasProrerty = definedType.weapProps["FF"].find(matchFolder) == definedType.weapProps["FF"].end() || getWeaponProp(weap, definedType.weapProps["FF"][matchFolder]) == true;
			if (hasCondition == false) {
				hasCondition = definedType.getCondiElement("FF", matchFolder) != "";
				hasLocal = false;
				Log1(" Checking Second time ("+ to_string( definedType.weapCondi["FF"].size() ) + ") " + matchFolder + " : property " + to_string(hasProrerty) + "; condition " + to_string(hasCondition));
			}

			return vector { hasProrerty, hasCondition, hasLocal };
		};

		TESForm* form = mIter.Get();
		if (form->IsWeapon()) {
			UInt8 currentIDX = form->GetModIndex();
			if (currentIDX == modIDX || modID == "FF") {
				string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(currentIDX));

				if (find(definedType.excluded.begin(), definedType.excluded.end(), fileModName) == definedType.excluded.end()) {
					auto weap = static_cast<TESObjectWEAP*>(form);
					weaponType weaponData = getWeaponData1(weap);
					vector<string> matchFolders;
					//Log1("Checking all files; ");
					matchFolders = weaponData.checkFolders2(definedType);
					for (int i = 0; i < matchFolders.size(); i++) {
						int randN = rand() % 100;
						srand(randN);
						Log1("\nScanning matched folder: " + matchFolders[i] + " with " + to_string(randN));
						auto folderChecks = checkAfolder(definedType, modID, weap, matchFolders[i]);
						if (folderChecks[0] == true) {
							if (folderChecks[1] == true) {
								weapBank.formAndFoldersWCondi[currentIDX][weaponData.hexedID].push_back(matchFolders[i]);
								if (folderChecks[2] == true) weapBank.formAndFoldersWCondi[currentIDX][weaponData.hexedID].push_back(definedType.weapCondi[modID][matchFolders[i]]);
								else weapBank.formAndFoldersWCondi[currentIDX][weaponData.hexedID].push_back(definedType.weapCondi["FF"][matchFolders[i]]);

								Log1(" Condition Found: " + definedType.weapCondi[modID][matchFolders[i]] + " for " + matchFolders[i]);
							}
							else if (weaponData.typeFolder != "") {
								Log1(" Replacement with property found: from " + weaponData.typeFolder + " to " + matchFolders[i]);
								if (randN > 50) weaponData.typeFolder = matchFolders[i];
							}
							else weaponData.typeFolder = matchFolders[i];
						}
						else if (folderChecks[1] == false && weaponData.typeFolder != "") {
							Log1(" Replacement found for " + weaponData.typeFolder);
							if (randN > 50) weaponData.typeFolder = matchFolders[i];
						}
					}
					
					if (weaponData.typeFolder != "") {
						Log1(weaponData.log1);
						Log1(" Replacing animations with " + weaponData.typeFolder + " for " + weaponData.name + "; refID " + weaponData.hexedID + "; mod ID " + FormatString(R"(%X)", weaponData.modIDXshort));
						weapBank.formAndFolder[currentIDX][weaponData.hexedID] = weaponData.typeFolder;
						weapBank.folderAndForm[currentIDX][weaponData.typeFolder].push_back(weaponData.hexedID);
						//Log1("Looking for conditions in " + modID);
					}
					else {
						weapBank.unassigned[weaponData.hexedID] = weaponData.typeParams;
					}
				}
			}
		}
	}

	return weapBank;
}

/*
weaponsBank processTypesMod1(vector<string>& weapType, map<string, vector<int>>& typesMap, vector<string> excluded, string modID)
{
	weaponsBank weapBank;

	NiTPointerMap<TESForm>* formsMap = *(NiTPointerMap<TESForm>**)0x11C54C0;
	int modIDX = HexStringToInt(modID);

	for (auto mIter = formsMap->Begin(); mIter; ++mIter) { // credits to Yvileapsis for the iteration example

		TESForm* form = mIter.Get();
		UInt8 currentIDX = form->GetModIndex();
		UInt32 currentIDXshort = currentIDX + (currentIDX * 0x00FFFFFF);
		string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(currentIDX));
		if (find(excluded.begin(), excluded.end(), fileModName) == excluded.end()) {
			if (currentIDX == modIDX || modID == "FF") {

				if (form->IsWeapon()) {
					auto weap = static_cast<TESObjectWEAP*>(form);
					string hexedID = FormatString(R"(%X)", weap->refID - currentIDXshort);
					string test = weap->GetName();

					Log1("Type to " + test + " : mod ID " + to_string(currentIDX) + " = " + fileModName);

					vector<int> weapParams;
					weapParams.push_back(weap->eWeaponType); //Animation type
					weapParams.push_back(weap->handGrip); //Grip type
					weapParams.push_back(weap->reloadAnim); //Reload animation (non-modded)
					weapParams.push_back(weap->attackAnim); //Attack animation
					Log1("Animation type: " + to_string(weapParams[0]));
					Log1("Grip type: " + to_string(weapParams[1]));
					Log1("Reload animation: " + to_string(weapParams[2]));
					Log1("Attack animation: " + to_string(weapParams[3]));
					// the log messages above are actually usefull for working with types
					// there's also a "reference" in tesEdit code

					for (int i = 0; i < weapType.size(); i++) {

						try {
							if (GetHasType2(weapParams, typesMap.at(weapType[i]))) {
								Log1("Replacing animations with " + weapType[i] + " for " + test + "; refID " + hexedID + "; mod ID " + FormatString(R"(%X)", currentIDXshort));
								weapBank.formAndFolder[currentIDX][hexedID] = weapType[i];

							}
						}
						catch (const out_of_range& oor) {
							Log1("This weapontype wasn't defined: " + weapType[i]);
						}
					}
				}
				else {
					//Log1("Type: wrong item " + to_string(traverse->key) + "; Current rem: " + to_string(traverse->key % formsMap->m_numBuckets));
				}
			}
		}
		else {
			//Log1("This mod is excluded: " + fileModName);
		}

	}

	return weapBank;
}
*/

json pushOrWrite(string prefix, weaponType& weaps, bool write = true, int reversed  = 0) {
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)";

	json subMod = json::object();
	//arrayWeaponsDB[formfolderPair.second] = json::array();
	//Log1("Registered2 form " + weaps.hexedID + ": " + weaps.fileModName + ": " + weaps.typeFolder);

	///write to json here
	subMod["mod"] = weaps.fileModName;
	subMod["form"] = weaps.hexedID;
	subMod["folder"] = weaps.typeFolder;

	if (write == true) {
		json rootArr = json::array();
		rootArr.push_back(subMod);
		if (reversed == 0) {
			Log1("Writing into: _" + weaps.fileModName + "_" + weaps.typeFolder + "_" + weaps.name + ".json");
			ofstream o(dir + prefix + weaps.fileModName + "_" + weaps.typeFolder + "_" + weaps.name + ".json");
			o << setw(4) << rootArr << endl;
		}
		else {
			Log1("Writing into: _" + weaps.typeFolder + "_" + weaps.fileModName + "_" + weaps.name + ".json");
			ofstream o(dir + prefix + weaps.typeFolder + "_" + weaps.fileModName + "_" + weaps.name + ".json");
			o << setw(4) << rootArr << endl;
		}
	}

	return subMod;
}



bool writeType(folderMap _map) {
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types\)";

	
	json rootArr = json::array();

	///write to json here
	
	for (auto params : _map.typeParams) {

		json subMod = params;
		rootArr.push_back({ { "folderType", _map.folderName },{"typedata", subMod } });
	}
	ofstream o(dir + "_types_agenerated_" + _map.folderName + ".json");
	o << setw(4) << rootArr << endl;
	Log1("Writing type for " + _map.folderName + " in _types_" + _map.folderName + ".json");
	
	

	return true;
}

bool writeType(vector<folderMap> _maps, string filename) {
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types\)";

	json rootArr = json::array();
	
	///write to json here
	for (auto _map : _maps) {
		for (auto params : _map.typeParams) {

			json subMod = params;
			rootArr.push_back({ { "folderType", _map.folderName },{"typedata", subMod } });
		}
		Log1("Writing type for " + _map.folderName + " in _types_" + filename + ".json");
	}

	ofstream o(dir + "_types_agenerated_" + filename + ".json");
	o << setw(4) << rootArr << endl;


	return true;
}

bool writeWeapList(string filename, string modID) {
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride)";
	

	json rootArr = json::array();

	const auto dirExclude = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types\_exclude.json)";
	if (filesystem::exists(dirExclude))
	{
		vector<string> EXCLUDED = {};
		ifstream i(dirExclude.c_str());
		json j;
		i >> j;
		for (auto& elem : j)
		{
			if (!elem.is_object())
			{
				Log1("JSON error: expected object with mod, form and folder fields");
				continue;
			}
			const auto& nameExclude = elem.contains("exclude_names") ? &elem["exclude_names"] : nullptr;
			if (nameExclude) {
				if (!nameExclude->is_array())
				{
					continue;
				}
				else {
					ranges::transform(*nameExclude, back_inserter(EXCLUDED), [&](auto& i) {return i.template get<string>(); });
					Log1("Reading name exclusions: " + EXCLUDED.back());
				}
			}

			EXCLUDED_NAMES = EXCLUDED;
		}
	}
	
	///write to json here
	json subMod = json::object();
	string fileFullPath = dir + R"(\_Types\_weapons_agenerated_)" + filename + ".json";
	if (filesystem::exists(dir))
	{
		if (modID != "FF") {
			int modIDX = HexStringToInt(modID);
			string ModName = FormatString("%s", DataHandler::Get()->GetNthModName(modIDX));
			subMod["mod"] = ModName;
			fileFullPath = dir + R"(\_Types\_weapons_agenerated_)" + filename + "_" + ModName +".json";
		}

		for (filesystem::directory_iterator iter(dir.c_str()), end; iter != end; ++iter)
		{
			const auto& path = iter->path();
			const auto& fileName = path.filename();
			const auto& SfileName = fileName.string();
			Log1("Checking " + SfileName);

			if ((path.extension().string().empty() || path.extension().string() == "") && SfileName.find(filename) != string::npos && checkName(SfileName, EXCLUDED_NAMES))
			{
				subMod["weapontypes"].push_back(SfileName);
			}
		}
	}
	rootArr.push_back(subMod);
	ofstream o(fileFullPath);
	o << setw(4) << rootArr << endl;


	return true;
}

int writeTypesFolders(string namePart) {
	aniMap themap;
	vector<folderMap> _maps;
	int numTypes = 0;

	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride)";
	if (filesystem::exists(dir))
	{
		for (filesystem::directory_iterator iter(dir.c_str()), end; iter != end; ++iter)
		{
			const auto& path = iter->path();
			const auto& fileName = path.filename();
			const auto& SfileName = fileName.string();
			Log1("Checking "+ SfileName);
			if (path.extension().string().empty() || path.extension().string() == "")
			{
				Log1("Extention " + path.extension().string() + " in " + SfileName + ", searching " + namePart);
				int pos = SfileName.find(namePart);

				//string sfilename;
				//transform(SfileName.begin(), SfileName.end(), std::back_inserter(sfilename), ::tolower);
				//int pos_empty = sfilename.find("empty");
				//int pos_lastshot = sfilename.find("lastshot");

				if (pos != string::npos && pos != -1 && checkName(SfileName, EXCLUDED_NAMES)) {
					folderMap scanResult;
					scanResult.getParams(themap, SfileName);
					_maps.push_back(scanResult);
					numTypes += scanResult.typeParams.size();
				}
			}

			Log1("Total " + to_string(numTypes)+" types;");
		}
	}
	writeType(_maps, namePart);
	return numTypes;
}

map<string, map<string, vector<int>>> DEEPscan(vector<int> weaponData) {
	aniMap themap;
	map<string, map<string, vector<int>>> _maps;
	int numTypes = 0;
	std::vector<std::thread> threads;
	const auto then = chrono::system_clock::now();

	auto scanAFolder = [](aniMap themap, string SfileName, vector<int>& weaponData, map<string, map<string, vector<int>>>& _maps) {
		folderMap scanResult;
		scanResult.getParams(themap, SfileName);

		_maps[SfileName] = scanResult.getWeaponScan(weaponData);
	};



	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride)";
	if (filesystem::exists(dir))
	{
		for (filesystem::directory_iterator iter(dir.c_str()), end; iter != end; ++iter)
		{
			
			const auto& path = iter->path();
			const auto& fileName = path.filename();
			const auto& SfileName = fileName.string();
			
			if (path.extension().string().empty() || path.extension().string() == "")
			{
				//Log1("Checking " + SfileName);

				threads.push_back( thread(scanAFolder,themap, SfileName, ref(weaponData), ref(_maps)) );
				
			}
		}
	}
	for (auto& th : threads) th.join();

	const auto now = chrono::system_clock::now();
	const auto diff = chrono::duration_cast<chrono::milliseconds>(now - then);
	Log1(FormatString("DEEP scanned in %d ms", diff.count()));
	return _maps;
}

bool writeUnassigned(weaponsBank& weapBanks) {
	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\_Types\)";

	json rootArr = json::array();
	json subMod = json::object();
	for (auto unass : weapBanks.unassigned) {
		subMod[unass.first] = { unass.second[0], unass.second[1], unass.second[2], unass.second[3] };
	}
	rootArr.push_back(subMod);
	Log1("Writing into: _unassigned_.json");
	ofstream o(dir + "_unassigned_.json");
	o << setw(4) << rootArr << endl;
}

bool writeJson(weaponsBank& weaponsDB, string prefix, string modIDX, int reversed = 0) {

	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)";

	Log1("writeJson start at: " + dir);

	if (!weaponsDB.formAndFolder.empty())
	{
		//regex regexp(".esp|.esm");
		if (modIDX == "FF") {
			for (auto formfolder : weaponsDB.formAndFolder)
			{
				string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(formfolder.first));
				//if (EXCLUDE.find(fileModName) == string::npos) {
				map<string, json> arrayWeaponsDB;
				map<int,string> separate;
				Log1("Writing to: " + fileModName);

				for (auto formfolderPair : formfolder.second)
				{
					json subMod = json::object();
					//arrayWeaponsDB[formfolderPair.second] = json::array();
					Log1("Registered2 form " + formfolderPair.first+ ", mod ID " + modIDX + ": " + fileModName + ": " + formfolderPair.second);

					///write to json here
					subMod["mod"] = fileModName;
					subMod["form"] = formfolderPair.first;
					subMod["folder"] = formfolderPair.second;

					arrayWeaponsDB[formfolderPair.second].push_back(subMod);

					//conditioned folders additively
					if (weaponsDB.formAndFoldersWCondi[formfolder.first].count(formfolderPair.first) == 1) {
						for (int i = 0; i < weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first].size(); i = i + 2)
						{
							//json subMod = json::object();
							Log1("Checking conditioned from " + weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first].size());

							Log1("Registered3 form " + formfolderPair.first + ", number " + to_string(i) + ": " + fileModName + ": " + weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first][i]);

							///write to json here
							subMod["mod"] = fileModName;
							subMod["form"] = formfolderPair.first;
							subMod["folder"] = weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first][i];
							subMod["condition"] = weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first][i + 1];

							arrayWeaponsDB[formfolderPair.second].push_back(subMod);
						}
					}

					//arrayWeaponsDB[formfolderPair.second].push_back(subMod);

					

				}
				//conditioned folders separately
				for (auto formCondifolderPair : weaponsDB.formAndFoldersWCondi[formfolder.first])
				{
					//json subMod = json::object();
					Log1("\n Checking conditioned from " + formCondifolderPair.second.size());
					//arrayWeaponsDB[formfolderPair.second] = json::array();
					for (int i = 0; i < formCondifolderPair.second.size(); i = i + 2) {
						if (separate.count(i) == 1) {
							Log1(FormatString("Registered4 form %d, number ", formCondifolderPair.first) + to_string(i) + ": " + fileModName + ": " + formCondifolderPair.second[i]);
							json subMod = json::object();
							///write to json here
							subMod["mod"] = fileModName;
							subMod["form"] = formCondifolderPair.first;
							subMod["folder"] = formCondifolderPair.second[i];
							subMod["condition"] = formCondifolderPair.second[i + 1];

							arrayWeaponsDB[formCondifolderPair.second[i]].push_back(subMod);
						}
					}
				}

				for (auto jsonarray : arrayWeaponsDB) {
					if (reversed == 0) {
						Log1("Writing into: _" + fileModName + "_" + jsonarray.first + ".json");
						ofstream o(dir + prefix + fileModName + "_" + jsonarray.first + ".json");
						o << setw(4) << jsonarray.second << endl;
					}
					else {
						Log1("Writing into: _" + jsonarray.first + "_" + fileModName + ".json");
						ofstream o(dir + prefix + jsonarray.first + "_" + fileModName + ".json");
						o << setw(4) << jsonarray.second << endl;
					}
				}
				//}
			}
		}
		else {
			int modIDXdec = HexStringToInt(modIDX);
			string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(modIDXdec));
			//if (EXCLUDE.find(fileModName) == string::npos) {
			map<string, json> arrayWeaponsDB;
			Log1("Writing to: " + fileModName);

			for (auto formfolderPair : weaponsDB.formAndFolder[modIDXdec])
			{
				json subMod = json::object();
				//arrayWeaponsDB[formfolderPair.second] = json::array();
				Log1(FormatString("Registered5 form %X, mod ID %d", formfolderPair.first, modIDXdec) + ": " + fileModName + ": " + formfolderPair.second);

				///write to json here
				subMod["mod"] = fileModName;
				subMod["form"] = formfolderPair.first;
				subMod["folder"] = formfolderPair.second;

				arrayWeaponsDB[formfolderPair.second].push_back(subMod);
			}

			for (auto formCondifolderPair : weaponsDB.formAndFoldersWCondi[modIDXdec])
			{
				json subMod = json::object();
				//arrayWeaponsDB[formfolderPair.second] = json::array();
				for (int i = 0; i < formCondifolderPair.second.size(); i = i + 2) {
					Log1(FormatString("Registered6 form %X, mod ID %d", formCondifolderPair.first, modIDXdec) + ": " + fileModName + ": " + formCondifolderPair.second[i]);

					///write to json here
					subMod["mod"] = fileModName;
					subMod["form"] = formCondifolderPair.first;
					subMod["folder"] = formCondifolderPair.second[i];
					subMod["condition"] = formCondifolderPair.second[i + 1];

					arrayWeaponsDB[formCondifolderPair.second[i]].push_back(subMod);
				}
			}
			for (auto jsonarray : arrayWeaponsDB) {
				//string fileModName1 = regex_replace(fileModName, regexp, "_");
				if (reversed == 0) {
					Log1("Writing into: _" + fileModName + "_" + jsonarray.first + ".json");
					ofstream o(dir + prefix + fileModName + "_" + jsonarray.first + ".json");
					o << setw(4) << jsonarray.second << endl;
				}
				else {
					Log1("Writing into: _" + jsonarray.first + "_" + fileModName + ".json");
					ofstream o(dir + prefix + jsonarray.first + "_" + fileModName + ".json");
					o << setw(4) << jsonarray.second << endl;
				}
			}
			//}
		}
	}
	else
	{
		Log1("There are no records here!");
	}
	return true;
}

bool writeJson2(weaponsBank& weaponsDB, string prefix, string modIDX, int reversed = 0) {

	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)";

	Log1("writeJson start at: " + dir);

	if (!weaponsDB.folderAndForm.empty())
	{
		for (auto formfolder : weaponsDB.folderAndForm)
		{
			string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(formfolder.first));
			//if (EXCLUDE.find(fileModName) == string::npos) {
			map<string, json> arrayWeaponsDB;
			map<int, string> separate;
			Log1("Writing to: " + fileModName);

			for (auto formfolderPair : formfolder.second)
			{
				json subMod = json::object();
				
				//arrayWeaponsDB[formfolderPair.second] = json::array();
				//string formInFolder(formfolderPair.second.begin(), formfolderPair.second.end());
				string formInFolder = accumulate(next(formfolderPair.second.begin()), formfolderPair.second.end(), formfolderPair.second[0],[](string a, string b) {
						return a + "," + b;
					}
				);

				Log1("Registered2 forms " + formInFolder + ", mod ID " + modIDX + ": " + fileModName + ": " + formfolderPair.first);

				///write to json here
				subMod["mod"] = fileModName;
				subMod["folder"] = formfolderPair.first;
				subMod["form"] = formfolderPair.second;
				arrayWeaponsDB[formfolderPair.first].push_back(subMod);

				for (auto folderIDX : formfolderPair.second) {
					//Log1("Checking from FORMS: " + folderIDX);
					if (weaponsDB.formAndFoldersWCondi[formfolder.first].count(folderIDX) == 1) {
						for (int i = 0; i < weaponsDB.formAndFoldersWCondi[formfolder.first][folderIDX].size(); i = i + 2)
						{
							//json subMod = json::object();
							json subModCond = json::object();
							Log1("Checking conditioned from " + weaponsDB.formAndFoldersWCondi[formfolder.first][folderIDX].size());

							Log1("Registered3 form " + folderIDX + ", number " + to_string(i) + ": " + fileModName + ": " + weaponsDB.formAndFoldersWCondi[formfolder.first][folderIDX][i]);

							///write to json here
							subModCond["mod"] = fileModName;
							subModCond["form"] = folderIDX;
							subModCond["folder"] = weaponsDB.formAndFoldersWCondi[formfolder.first][folderIDX][i];
							subModCond["condition"] = weaponsDB.formAndFoldersWCondi[formfolder.first][folderIDX][i + 1];

							arrayWeaponsDB[formfolderPair.first].push_back(subModCond);
						}
					}
				}

				//conditioned folders additively
				if (weaponsDB.formAndFoldersWCondi[formfolder.first].count(formfolderPair.first) == 1) {
					for (int i = 0; i < weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first].size(); i = i + 2)
					{
						//json subMod = json::object();
						json subModCond = json::object();
						Log1("Checking conditioned from " + weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first].size());

						Log1("Registered3 form " + formfolderPair.first + ", number " + to_string(i) + ": " + fileModName + ": " + weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first][i]);

						///write to json here
						subModCond["mod"] = fileModName;
						subModCond["form"] = formfolderPair.first;
						subModCond["folder"] = weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first][i];
						subModCond["condition"] = weaponsDB.formAndFoldersWCondi[formfolder.first][formfolderPair.first][i + 1];

						arrayWeaponsDB[formfolderPair.first].push_back(subModCond);
					}
				}

				//arrayWeaponsDB[formfolderPair.second].push_back(subMod);



			}
			//conditioned folders separately
			for (auto formCondifolderPair : weaponsDB.formAndFoldersWCondi[formfolder.first])
			{
				//json subMod = json::object();
				Log1("\n Checking conditioned from " + formCondifolderPair.second.size());
				//arrayWeaponsDB[formfolderPair.second] = json::array();
				for (int i = 0; i < formCondifolderPair.second.size(); i = i + 2) {
					if (separate.count(i) == 1) {
						Log1(FormatString("Registered4 form %d, number ", formCondifolderPair.first) + to_string(i) + ": " + fileModName + ": " + formCondifolderPair.second[i]);
						json subMod = json::object();
						///write to json here
						subMod["mod"] = fileModName;
						subMod["form"] = formCondifolderPair.first;
						subMod["folder"] = formCondifolderPair.second[i];
						subMod["condition"] = formCondifolderPair.second[i + 1];

						arrayWeaponsDB[formCondifolderPair.second[i]].push_back(subMod);
					}
				}
			}

			for (auto jsonarray : arrayWeaponsDB) {
				if (reversed == 0) {
					Log1("Writing into: _" + fileModName + "_" + jsonarray.first + ".json");
					ofstream o(dir + prefix + fileModName + "_" + jsonarray.first + ".json");
					o << setw(4) << jsonarray.second << endl;
				}
				else {
					Log1("Writing into: _" + jsonarray.first + "_" + fileModName + ".json");
					ofstream o(dir + prefix + jsonarray.first + "_" + fileModName + ".json");
					o << setw(4) << jsonarray.second << endl;
				}
			}
			//}
		}
	}
	else
	{
		Log1("There are no records here!");
	}
	return true;
}

json writeAweapon(string prefix, TESObjectWEAP* weap, int reversed, string folder) {
	vector<int> params;
	typesBank typesDB = readTypes(params);
	weaponType weaps = getWeaponData1(weap);
	vector<string> matchFolders = weaps.checkFolders3(typesDB);
	if (folder != "_") {
		for (auto aFolder : matchFolders) {
			Log1("\n Checking " + aFolder + " vs " + folder);
			if (aFolder == folder) weaps.typeFolder = folder;
		}
	}
	else {
		if (matchFolders.size() == 1) {
			weaps.typeFolder = matchFolders[0];
		}
		else if (matchFolders.size() > 1) {
			srand(time(NULL));
			weaps.typeFolder = matchFolders[rand() % matchFolders.size()];
		}
	}
	if (weaps.typeFolder == "") {
		Console_Print(FormatString("No matching folders found, first set: %s ", matchFolders[0]).c_str());
		weaps.typeFolder = matchFolders[0];
	}
	return pushOrWrite(prefix, weaps, true, reversed);
}

json writeAweapon2(string prefix, TESObjectWEAP* weap, int reversed, string folder) {
	weaponType weaps = getWeaponData1(weap);
	map<string, map<string, vector<int>>> scanFull = DEEPscan(weaps.typeParams);
	vector<string> matchFolders;
	for (auto aFolder : scanFull) {
		if (aFolder.second["matched"][0] != 0 && aFolder.second["matched"][1] != 0 && aFolder.second["matched"][3] != 0 && aFolder.second["matched"][4] != 0) {
			matchFolders.push_back(aFolder.first);
		}
	}
	if (folder != "_") {
		for (auto aFolder : matchFolders) {
			Log1("\n Checking " + aFolder + " vs " + folder);
			if (aFolder == folder) weaps.typeFolder = folder;
		}
	}
	else {
		if (matchFolders.size() == 1) {
			weaps.typeFolder = matchFolders[0];
		}
		else if (matchFolders.size() > 1) {
			srand(time(NULL));
			weaps.typeFolder = matchFolders[rand() % matchFolders.size()];
		}
	}
	if (weaps.typeFolder == "") {
		Console_Print(FormatString("No matching folders found, first set: %s ", matchFolders[0]).c_str());
		weaps.typeFolder = matchFolders[0];
	}
	return pushOrWrite(prefix, weaps, true, reversed);
}

vector<string> matchAWeapon(TESObjectWEAP* weap) {

	vector<int> params;
	typesBank typesDB = readTypes(params);
	weaponType weaps = getWeaponData1(weap);
	return weaps.checkFolders3(typesDB);

}

vector<string> matchAWeapon1(weaponType& weaps, vector<int> param1) {

	vector<int> params;
	typesBank typesDB = readTypes(params);
	if (param1.empty()) {
		return weaps.checkFolders3(typesDB);
	}
	else {
		vector<string> matching;

		Log1(" Checking by defined params ");
		for (int i = 0; i < typesDB.typesMap1.size(); i++) {
			if (GetHasType2(typesDB.typesMap2[i], param1)) {
				matching.push_back(typesDB.typesMap1[i] + " [ " + to_string(typesDB.typesMap2[i][0]) + " " + to_string(typesDB.typesMap2[i][1]) + " " + to_string(typesDB.typesMap2[i][2]) + " " + to_string(typesDB.typesMap2[i][3]) + " ]");
			}
		}

		return matching;
	}

}

bool processTypesAndWrite2(string prefix, string modIDX, int eWeaponType, int handGrip, int reloadAnim, int attackAnim, int reversed, string typesfilename, string weapsfilename) {
	const auto then = chrono::system_clock::now();
	
	vector<int> params;
	if (!(eWeaponType == 0 && handGrip == 0 && reloadAnim == 0 && attackAnim == 0)) {
		
		params = { eWeaponType , handGrip , reloadAnim , attackAnim };
		Log1(FormatString("Params are %d, %d, %d and %d", params[0], params[1], params[2], params[3]));
	}

	typesBank typesDB = readTypes(params, typesfilename, weapsfilename);
	if (reversed == 0 && find(typesDB.options.begin(), typesDB.options.end(), "reversed") != typesDB.options.end()) reversed = 1;

	if (typesDB.weapType.empty()) {
		return false;
	}
	else if (modIDX == "FF") {
		Log1("Starting FF");
		//weaponsBank weaps = processTypesMod1(mods.second, typesDB.typesMap, typesDB.excluded, modIDX);
		weaponsBank weaps = processTypesMod2(typesDB, modIDX);
		writeJson2(weaps, prefix, modIDX, reversed);
		writeUnassigned(weaps);
	}
	else {
		Log1("Starting a single mod: " + modIDX);
		//weaponsBank weaps = processTypesMod1(mods.second, typesDB.typesMap, typesDB.excluded, modIDX);
		weaponsBank weaps = processTypesMod2(typesDB, modIDX);
		writeJson2(weaps, prefix, modIDX, reversed);
		writeUnassigned(weaps);
	}
	const auto now = chrono::system_clock::now();
	const auto diff = chrono::duration_cast<chrono::milliseconds>(now - then);
	Log1(FormatString("Written jsons in %d ms", diff.count()));

	return true;
}

//////////////////////////////////

//bool writeJson2(weaponsBank weaponsDB, string prefix, string modIDX) {
//
//	const auto dir = GetCurPath() + R"(\Data\Meshes\AnimGroupOverride\)";
//
//	Log1("writeJson start at: " + dir);
//
//	if (!weaponsDB.formAndFolder.empty())
//	{
//			int modIDXdec = HexStringToInt(modIDX);
//			string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(modIDXdec));
//			//if (EXCLUDE.find(fileModName) == string::npos) {
//			map<string, json> arrayWeaponsDB;
//			Log1("Writing to: " + fileModName);
//
//			for (auto formfolderPair : weaponsDB.formAndFolder[modIDXdec])
//			{
//				json subMod = json::object();
//				//arrayWeaponsDB[formfolderPair.second] = json::array();
//				Log1(FormatString("Registered2 form %X, mod ID %d", formfolderPair.first, modIDXdec) + ": " + fileModName + ": " + formfolderPair.second);
//
//				///write to json here
//				subMod["mod"] = fileModName;
//				subMod["form"] = formfolderPair.first;
//				subMod["folder"] = formfolderPair.second;
//				arrayWeaponsDB[formfolderPair.second].push_back(subMod);
//			}
//			for (auto jsonarray : arrayWeaponsDB) {
//				regex regexp(".esp+|.esm+");
//				regex_replace(fileModName, regexp, "_");
//				Log1("Writing into: _" + fileModName + "_" + jsonarray.first + ".json");
//				ofstream o(dir + prefix + fileModName + "_" + jsonarray.first + ".json");
//				o << setw(4) << jsonarray.second << endl;
//			}
//	}
//	else
//	{
//		Log1("There are no records here!");
//	}
//	return true;
//}

//bool processTypesAndWrite(string prefix, string modIDX) {
//	const auto then = chrono::system_clock::now();
//	vector<int> params;
//	typesBank typesDB = readTypes(params);
//	if (typesDB.weapType.empty()) {
//		return false;
//	}
//	else if (modIDX == "FF") {
//		Log1("Starting GLOBAL");
//		//weaponsBank weaps = processTypesMod1(mods.second, typesDB.typesMap, typesDB.excluded, modIDX);
//		weaponsBank weaps = processTypesMod2(typesDB, modIDX);
//		writeJson(weaps, prefix, modIDX);
//	}
//	else {
//		Log1("Starting a single mod: " + modIDX);
//		//weaponsBank weaps = processTypesMod1(mods.second, typesDB.typesMap, typesDB.excluded, modIDX);
//		weaponsBank weaps = processTypesMod2(typesDB, modIDX);
//		writeJson(weaps, prefix, modIDX);
//	}
//	const auto now = chrono::system_clock::now();
//	const auto diff = chrono::duration_cast<chrono::milliseconds>(now - then);
//	Log1(FormatString("Written jsons in %d ms", diff.count()));
//
//	return true;
//}
