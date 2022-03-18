#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/GameObjects.h"
#include "file_addon.h"
#include <string>

//NoGore is unsupported in this fork

#ifndef RegisterScriptCommand
#define RegisterScriptCommand(name) 	nvse->RegisterCommand(&kCommandInfo_ ##name);
#endif

IDebugLog		gLog("knvse_jsongenerator.log");

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface;
NVSEInterface* g_nvseInterface;
NVSECommandTableInterface* g_cmdTable;
NVSEArrayVarInterface* dataArr;
const CommandInfo* g_TFC;
typedef void (*PF)();
std::map<std::string, PF> defined_functions;

static ParamInfo kParams_OneString_ThreeOptionalStringsFiveOptionalInts[9] =
{
	{	"string",		kParamType_String,			0	},
	{	"string",			kParamType_String,	1	},
	{	"string",			kParamType_String,	1	},
	{	"string",			kParamType_String,	1	},
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
};

static ParamInfo kParams_OneOptionalString_FourOptionalInts[5] =
{
	{	"string",		kParamType_String,			1	},
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
};

static ParamInfo kParams_FourOptionalInts[4] =
{
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
};

static ParamInfo kParams_FiveOptionalInts[5] =
{
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
	{	"int", kParamType_Integer, 1 },
};

static ParamInfo kParams_OneString_OneOptionalString[2] =
{
	{	"string",			kParamType_String,	0	},
	{	"string",			kParamType_String,	1	}
};

static ParamInfo kParams_OneString_OneOptionalStringOneOptionalInt[3] =
{
	{	"string",			kParamType_String,	0	},
	{	"string",			kParamType_String,	1	},
	{	"int", kParamType_Integer, 1 },
};

static ParamInfo kParams_OneString_OneOptionalInt[2] =
{
	{	"string",			kParamType_String,	0	},
	{	"int", kParamType_Integer, 1 },
};

#if RUNTIME
NVSEScriptInterface* g_script;
#endif
// This is a message handler for nvse events
// With this, plugins can listen to messages such as whenever the game loads
void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case NVSEMessagingInterface::kMessage_DeferredInit:
		_MESSAGE("kMessage_DeferredInit TEST");
		break;
	case NVSEMessagingInterface::kMessage_PostLoad:
		_MESSAGE("kMessage_PostLoad TEST");
		break;
	default:
		break;
	}
}

bool Cmd_knvse_generate_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_generate_Execute(COMMAND_ARGS)
{
	_MESSAGE("Generating files");

	
	char pref[0x1000];
	std::string modIDX = "FF";
	char typesfilename[0x1000] = "_";
	char weapsfilename[0x1000] = "_";
	int eWeaponType = 00;
	int handGrip = 000;
	int reloadAnim = 00;
	int attackAnim = 00;
	int reversed = 0;

	if (ExtractArgs(EXTRACT_ARGS, &pref, &modIDX, &typesfilename, &weapsfilename, &eWeaponType, &handGrip, &reloadAnim, &attackAnim, &reversed))
	{
		if (modIDX == "FF")
		{
			Console_Print(FormatString("Generating for all: folders %s and %s", typesfilename, weapsfilename).c_str());
		}
		else {
			Console_Print(FormatString("Generating for mod index %s, folders %s and %s", eWeaponType, typesfilename, weapsfilename).c_str());
		}
		processTypesAndWrite2(pref, modIDX, eWeaponType, handGrip, reloadAnim, attackAnim, reversed, typesfilename, weapsfilename);
	}

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_generate, "Generates .json files for all the weapons that fit into types - with parameters.", 0, 9, kParams_OneString_ThreeOptionalStringsFiveOptionalInts)
/// prefix, mod ID (hex), first folder (usually types), second folder (usually weapons), 4 weapon parameters, reversed or not (priority over options in .json) 

bool Cmd_knvse_getfolders_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_getfolders_Execute(COMMAND_ARGS)
{
	_MESSAGE("Reading weapon data...");
	PlayerCharacter* npc = g_thePlayer->GetSingleton();

	Actor* actor = static_cast<Actor*>(npc);
	auto* weap = actor->GetWeaponForm();
	int eWeaponType = 00;
	int handGrip = 000;
	int reloadAnim = 00;
	int attackAnim = 00;

	weaponType weaparams = getWeaponData1(weap);
	Console_Print(FormatString("Parameters: %d %d %d %d", weaparams.typeParams[0], weaparams.typeParams[1], weaparams.typeParams[2], weaparams.typeParams[3]).c_str());

	if (ExtractArgs(EXTRACT_ARGS, &eWeaponType, &handGrip, &reloadAnim, &attackAnim)) {
		if (eWeaponType == 00 && handGrip == 000 && reloadAnim == 00 && attackAnim == 00) {
			vector<string> folders = matchAWeapon1(weaparams);
			for (auto folder : folders) {
				Console_Print(folder.c_str());
			}
		}
		else {
			Console_Print(FormatString("Input instead: %d %d %d %d", eWeaponType, handGrip, reloadAnim, attackAnim).c_str());
			vector<string> folders = matchAWeapon1(weaparams, { eWeaponType, handGrip, reloadAnim, attackAnim });
			for (auto folder : folders) {
				Console_Print(folder.c_str());
			}
		}
	}

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_getfolders, "Prints folders suited for the equipped weapon", 0, 4, kParams_FourOptionalInts)

bool Cmd_knvse_generateone_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_generateone_Execute(COMMAND_ARGS)
{
	_MESSAGE("Reading weapon data...");
	PlayerCharacter* npc = g_thePlayer->GetSingleton();
	string prefix = "aaa_";
	int reversed = 0;
	char folder[0x1000] = "_";
	

	if (ExtractArgs(EXTRACT_ARGS, &prefix, &folder, &reversed))
	{

		Actor* actor = static_cast<Actor*>(npc);
		auto* weap = actor->GetWeaponForm();

		weaponType weaparams = getWeaponData1(weap);
		nlohmann::json result = writeAweapon(prefix, weap, reversed, folder);

		Console_Print(FormatString("Writing in: %s .json", prefix + weaparams.fileModName + folder).c_str());

	}

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_generateone, "Writes a json for the equipped weapon", 0, 4, kParams_OneString_OneOptionalStringOneOptionalInt)
// Prefix, folder, reversed or not

bool Cmd_knvse_scan_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_scan_Execute(COMMAND_ARGS)
{
	_MESSAGE("Reading weapon data...");
	char folder[0x1000];
	aniMap themap;
	folderMap scanResult;

	if (ExtractArgs(EXTRACT_ARGS, &folder)) {
		scanResult.getParams(themap, folder);
		for (auto data : scanResult.subMaps) {
			Console_Print(("Reloads for " + data.type + ": " + data.Sreloads).c_str());
			Console_Print(("Attacks for " + data.type + ": " + data.Sattacks).c_str());
			
		}
		Console_Print((to_string(scanResult.typeParams.size()) + " combination(s) total ").c_str());
	}

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_scan, "Scans folders for data", 0, 1, kParams_OneString)

bool Cmd_knvse_deepscan_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_deepscan_Execute(COMMAND_ARGS)
{
	_MESSAGE("Reading weapon data...");
	PlayerCharacter* npc = g_thePlayer->GetSingleton();

	Actor* actor = static_cast<Actor*>(npc);
	auto* weap = actor->GetWeaponForm();
	int eWeaponType = 00;
	int handGrip = 000;
	int reloadAnim = 00;
	int attackAnim = 00;
	char moreIfnfo = 'f';

	weaponType weaparams = getWeaponData1(weap);
	

	if (ExtractArgs(EXTRACT_ARGS, &moreIfnfo, &eWeaponType, &handGrip, &reloadAnim, &attackAnim)) {
		map<string, map<string, vector<int>>> dataMap;

		if (eWeaponType == 00 && handGrip == 000 && reloadAnim == 00 && attackAnim == 00) {
			Console_Print(FormatString("Parameters: %d %d %d %d", weaparams.typeParams[0], weaparams.typeParams[1], weaparams.typeParams[2], weaparams.typeParams[3]).c_str());
			dataMap = DEEPscan(weaparams.typeParams);
		} else {
			dataMap = DEEPscan({ eWeaponType, handGrip, reloadAnim, attackAnim });
			Console_Print(FormatString("Parameters: %d %d %d %d", eWeaponType, handGrip, reloadAnim, attackAnim).c_str());
		}

		for (auto folder : dataMap) {
			if (moreIfnfo != 'a' && moreIfnfo != 'r') {
				if (folder.second["matched"][0] != 0 && folder.second["matched"][1] != 0 && folder.second["matched"][3] != 0 && folder.second["matched"][4] != 0) {
					Log1((folder.first + ": full match [" + to_string(folder.second["matched"][0]) + " " + to_string(folder.second["matched"][1]) + " " + to_string(folder.second["matched"][2]) + " " + to_string(folder.second["matched"][3]) + "] ").c_str());
					Console_Print((folder.first + ": full match [" + to_string(folder.second["matched"][0]) + " " + to_string(folder.second["matched"][1]) + " " + to_string(folder.second["matched"][2]) + " " + to_string(folder.second["matched"][3]) + "] ").c_str());
				}
			}
			if (moreIfnfo != 'a' && moreIfnfo != 'f') {
				if (folder.second["reload"].size() > 0) {
					string reloads;
					for (auto reload : folder.second["reload"]) {
						reloads += " " + to_string(reload);
					}
					Log1((folder.first + ": only reload, attacks: [" + reloads + "] ").c_str());
					Console_Print((folder.first + ": only reload, attacks: [" + reloads + "] ").c_str());
				}
			}
			if (moreIfnfo != 'r' && moreIfnfo != 'f') {
				if (folder.second["attack"].size() > 0)
				{
					string attacks;
					for (auto reload : folder.second["attack"]) {
						attacks += " " + to_string(reload);
					}
					Console_Print((folder.first + ": only attack, reloads: [" + attacks + "] ").c_str());
					Log1((folder.first + ": only attack, reloads: [" + attacks + "] ").c_str());
				}
			}
		}
	}
	else Console_Print("f is full match only, a is attacks, r is reloads, anything else is all. The rest is four parameters");

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_deepscan, "Scans all the folders for animations and looks for mathing parameters", 0, 5, kParams_OneOptionalString_FourOptionalInts)

bool Cmd_knvse_generatetypes_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_generatetypes_Execute(COMMAND_ARGS)
{
	_MESSAGE("Reading weapon data...");
	char folder[0x1000];
	int many = 0;
	aniMap themap;
	folderMap scanResult;

	if (ExtractArgs(EXTRACT_ARGS, &folder, &many)) {
		if (many == 0) {
			scanResult.getParams(themap, folder);
			writeType(scanResult);
			Console_Print(("Generated " + to_string(scanResult.typeParams.size()) + " types ").c_str());
		}
		else {
			int numTypes = writeTypesFolders(folder);
			Console_Print(("Generated many, " + to_string(numTypes) + " types ").c_str());
		}
	}

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_generatetypes, "Generates types file for a folder", 0, 2, kParams_OneString_OneOptionalInt)

bool Cmd_knvse_generatelist_Execute(COMMAND_ARGS);

#if RUNTIME
//In here we define a script function
//Script functions must always follow the Cmd_FunctionName_Execute naming convention
bool Cmd_knvse_generatelist_Execute(COMMAND_ARGS)
{
	_MESSAGE("Reading weapon data...");
	char folder[0x1000];
	string modID = "FF";

	if (ExtractArgs(EXTRACT_ARGS, &folder, &modID)) {

		writeWeapList(folder, modID);

		Console_Print(FormatString("Writing weapons list of all with %s for a modID %s ",folder, modID).c_str());
	}

	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(knvse_generatelist, "Generates types file for a folder", 0, 2, kParams_OneString_OneOptionalString)


bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");


	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MyFirstPlugin";
	info->version = 2;

	// version checks
	if (nvse->nvseVersion < NVSE_VERSION_INTEGER)
	{
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, NVSE_VERSION_INTEGER);
		return false;
	}

	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}

		if (nvse->isNogore)
		{
			_ERROR("NoGore is not supported");
			return false;
		}
	}

	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	}

	// version checks pass
	// any version compatibility checks should be done here
	return true;
}

bool NVSEPlugin_Load(const NVSEInterface* nvse)
{
	_MESSAGE("load");

	g_pluginHandle = nvse->GetPluginHandle();

	// save the NVSEinterface in cas we need it later
	g_nvseInterface = (NVSEInterface*)nvse;

	// register to receive messages from NVSE
	g_messagingInterface = (NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging);
	g_messagingInterface->RegisterListener(g_pluginHandle, "NVSE", MessageHandler);

	

#if RUNTIME
	g_script = (NVSEScriptInterface*)nvse->QueryInterface(kInterface_Script);
#endif
	/***************************************************************************
	 *
	 *	READ THIS!
	 *
	 *	Before releasing your plugin, you need to request an opcode range from
	 *	the NVSE team and set it in your first SetOpcodeBase call. If you do not
	 *	do this, your plugin will create major compatibility issues with other
	 *	plugins, and will not load in release versions of NVSE. See
	 *	nvse_readme.txt for more information.
	 *
	 *	See https://geckwiki.com/index.php?title=NVSE_Opcode_Base
	 *
	 **************************************************************************/

	 // register commands
	nvse->SetOpcodeBase(0x3DDD);
	//RegisterScriptCommand(knvse_typegen);
	RegisterScriptCommand(knvse_generate);
	RegisterScriptCommand(knvse_getfolders);
	RegisterScriptCommand(knvse_generateone);
	RegisterScriptCommand(knvse_scan);
	RegisterScriptCommand(knvse_deepscan);
	RegisterScriptCommand(knvse_generatetypes);
	RegisterScriptCommand(knvse_generatelist);

	

	return true;
}
