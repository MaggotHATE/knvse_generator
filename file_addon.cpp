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
			if (_stricmp(path.extension().string().c_str(), ".json") == 0)
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
										Log1("Filename = " + SfileName);
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
									string logloc = "Reading inside: \n";
									ranges::transform(*types, back_inserter(typesDB.weapType[modChecked]), [&](auto& i) {
										string iString = i.template get<string>();
										int condiPos = findString(iString, "+condition");
										int propPos = findString(iString, "+properties");

										if (condiPos == 999 && propPos == 999) {
											logloc += ("\n Pure folder name: "+ iString);
											return iString;
										}
										else {
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
											}


											if (iStringDataCondi != "") {
												logloc += ("\n Conditions: " + iStringDataCondi);
												//logloc +=("\n Properties: " + iStringDataProp);
												typesDB.weapCondi[modChecked][iStringName] = iStringDataCondi;
												//typesDB.weapProps[modChecked][iStringName] = iStringDataProp;
												if (condiPos != 999);
												if (propPos != 999);
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
	Log1("Weapon data for " + getweapon.name + "(" + getweapon.hexedID + ")" + " : mod ID " + FormatString(R"(%X)", getweapon.modIDX) + " = " + getweapon.fileModName);

	getweapon.typeParams.push_back(weap->eWeaponType); //Animation type
	getweapon.typeParams.push_back(weap->handGrip); //Grip type
	getweapon.typeParams.push_back(weap->reloadAnim); //Reload animation (non-modded)
	getweapon.typeParams.push_back(weap->attackAnim); //Attack animation
	//Log1("Animation type: " + to_string(getweapon.typeParams[0]));
	//Log1("Grip type: " + to_string(getweapon.typeParams[1]));
	//Log1("Reload animation: " + to_string(getweapon.typeParams[2]));
	//Log1("Attack animation: " + to_string(getweapon.typeParams[3]));

	getweapon.log1 += "\n";
	getweapon.log1 += ("Animation type: " + to_string(getweapon.typeParams[0]));
	getweapon.log1 += ("Grip type: " + to_string(getweapon.typeParams[1]));
	getweapon.log1 += ("Reload animation: " + to_string(getweapon.typeParams[2]));
	getweapon.log1 += ("Attack animation: " + to_string(getweapon.typeParams[3]));
	getweapon.log1 += "\n";


	return getweapon;
}

weaponsBank processTypesMod2(typesBank& definedType, string modID)
{
	weaponsBank weapBank;

	NiTPointerMap<TESForm>* formsMap = *(NiTPointerMap<TESForm>**)0x11C54C0;
	int modIDX = HexStringToInt(modID);

	for (auto mIter = formsMap->Begin(); mIter; ++mIter) { // credits to Yvileapsis for the iteration example

		TESForm* form = mIter.Get();
		if (form->IsWeapon()) {
			UInt8 currentIDX = form->GetModIndex();
			if (currentIDX == modIDX || modID == "FF") {
				string fileModName = FormatString("%s", DataHandler::Get()->GetNthModName(currentIDX));

				if (find(definedType.excluded.begin(), definedType.excluded.end(), fileModName) == definedType.excluded.end()) {
					auto weap = static_cast<TESObjectWEAP*>(form);
					weaponType weaponData = getWeaponData1(weap);
					vector<string> matchFolders;
					//if (typesfilename == "") {
						Log1("Checking all files; ");
						matchFolders = weaponData.checkFolders2(definedType);
					//}
					//else {
					//	Log1("Checking a file: " + typesfilename);
					//	matchFolders = weaponData.checkFoldersFile1(definedType, typesfilename);
					//}
					if (matchFolders.size() >= 1) {
						weaponData.typeFolder = matchFolders[0];
					}
					if (matchFolders.size() > 1) {
						srand(time(NULL));
						for (auto folder : matchFolders){
							if (definedType.weapCondi[modID].find(folder) != definedType.weapCondi[modID].end()) {
								weapBank.formAndFoldersWCondi[currentIDX][weaponData.hexedID].push_back(folder);
								weapBank.formAndFoldersWCondi[currentIDX][weaponData.hexedID].push_back(definedType.weapCondi[modID][folder]);

								Log1("Condition Found: " + definedType.weapCondi[modID][folder] + " for "+ folder);
							}
							else if (rand()% 100 < 50){
								weaponData.typeFolder = folder;
							}
						}
					}
					if (weaponData.typeFolder != "") {
						Log1(weaponData.log1);
						Log1("Replacing animations with " + weaponData.typeFolder + " for " + weaponData.name + "; refID " + weaponData.hexedID + "; mod ID " + FormatString(R"(%X)", weaponData.modIDXshort));
						weapBank.formAndFolder[currentIDX][weaponData.hexedID] = weaponData.typeFolder;
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

json writeAweapon(string prefix, TESObjectWEAP* weap, int reversed, string folder) {
	vector<int> params;
	typesBank typesDB = readTypes(params);
	weaponType weaps = getWeaponData1(weap);
	if (folder != "_") weaps.typeFolder = folder;
	else {
		vector<string> matchFolders = weaps.checkFolders3(typesDB);
		if (matchFolders.size() == 1) {
			weaps.typeFolder = matchFolders[0];
		}
		else if (matchFolders.size() > 1) {
			srand(time(NULL));
			weaps.typeFolder = matchFolders[rand() % matchFolders.size()];
		}
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
		writeJson(weaps, prefix, modIDX, reversed);
		writeUnassigned(weaps);
	}
	else {
		Log1("Starting a single mod: " + modIDX);
		//weaponsBank weaps = processTypesMod1(mods.second, typesDB.typesMap, typesDB.excluded, modIDX);
		weaponsBank weaps = processTypesMod2(typesDB, modIDX);
		writeJson(weaps, prefix, modIDX, reversed);
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
