#include <Windows.h>
#include <RE/Bethesda/SendPapyrusEvent.h>

// Functions to save and restore game settings
// Mantella temporarily modifies some settings to mute some NPC commentary
// During player conversation

RE::GameSettingCollection* GameSettings;

std::map<std::string, float> SettingsFloat;
std::map<std::string, int> SettingsInt;

void initSettings() {
	GameSettings = RE::GameSettingCollection::GetSingleton();
	}

bool saveFloat(std::monostate, std::string key) {
	bool ret = false;
	if (GameSettings) {
		RE::Setting* floatsetting = GameSettings->GetSetting(key);
		if (floatsetting != NULL && floatsetting->GetType() == RE::Setting::SETTING_TYPE::kFloat) {
			SettingsFloat.insert_or_assign(key, floatsetting->GetFloat());
			ret = true;
			}
		}
	return ret;
	}

bool saveInt(std::monostate, std::string key) {
	bool ret = false;
	if (GameSettings) {
		RE::Setting* intsetting = GameSettings->GetSetting(key);
		if (intsetting != NULL && intsetting->GetType() == RE::Setting::SETTING_TYPE::kInt) {
			SettingsInt.insert_or_assign(key, intsetting->GetInt());
			ret = true;
			}
		}
	return ret;
	}

bool restoreFloat(std::monostate, std::string key) {
	bool ret = false;
	if (GameSettings) {
		RE::Setting* floatsetting = GameSettings->GetSetting(key);
		if (SettingsFloat.contains(key) && floatsetting != NULL) {
			floatsetting->SetFloat(SettingsFloat.at(key));
			ret = true;
			}
		}
	return ret;
	}

bool restoreInt(std::monostate, std::string key) {
	bool ret = false;
	if (GameSettings) {
		RE::Setting* intsetting = GameSettings->GetSetting(key);
		if (SettingsInt.contains(key) && intsetting != NULL) {
			intsetting->SetInt(SettingsInt.at(key));
			ret = true;
			}
		}
	return ret;
	}

extern void RegisterCrosshair();
extern RE::TESFormID GetLastCrossHairActorID(std::monostate);
extern std::vector<float> GetLastActorCoords(std::monostate);


// Substitute for SUP_F4SE's function
bool isMenuModeActive(std::monostate) {
	RE::UI* pUI = RE::UI::GetSingleton();
	return pUI->GetMenuOpen("DialogueMenu")
		|| pUI->GetMenuOpen("ExamineMenu")
		|| pUI->GetMenuOpen("PowerArmorModMenu")
		|| pUI->GetMenuOpen("PipboyMenu")
		|| pUI->GetMenuOpen("VATSMenu")
		|| pUI->GetMenuOpen("LooksMenu")
		|| pUI->GetMenuOpen("BarterMenu")
		|| pUI->GetMenuOpen("WorkshopMenu")
		|| pUI->GetMenuOpen("Console")
		|| pUI->GetMenuOpen("SpecialMenu")
		|| pUI->GetMenuOpen("SimpleTextField");
	}

void CopyFuz(std::string fname) {							// Copy the generated FUZ file to the requested name
	char exe_path[_MAX_PATH];

	GetModuleFileNameA(NULL, exe_path, _MAX_PATH);			// Fallout exe
	std::filesystem::path app_path(exe_path);
	std::filesystem::path base_path(app_path.remove_filename());
	base_path += "Data\\Sound\\Voice\\Mantella.esp\\MantellaVoice00";		// Path to Mantella voices directory
	std::filesystem::path src_path(base_path / "00001ED2_1.fuz");
	std::filesystem::path target_file(base_path / std::format("{}_1.fuz", fname));
	CopyFile(src_path.c_str(), target_file.c_str(), false);
	}

// Patch Response text in memory for a given Topic
int PatchTopicInfo(std::monostate, RE::TESTopic* topic, std::string new_text) {
	int ret = 0;

	if (topic->numTopicInfos == 1) {							// Sanity check, to avoid overwriting the wrong location
		RE::TESTopicInfo* tpInfo = topic->topicInfos[0];		// Get the TopicInfo from Topic arg
		RE::ResponseListWrapper resp_list = tpInfo->responses;
		RE::TESResponse* response = resp_list.head;

		char* resp_text = (char*)response->responseText.c_str();	// Existing response text

		if (new_text.length() > 450) {
			ret = -2;
			logger::warn("Replacement too long! ({:d}", new_text.length());
			}
		else {
			memset(resp_text, 0, 451);					// Fill w/0
			strcpy_s(resp_text, 450, new_text.data());	// Copy new text for subtittles
			}

		if (ret != 0) {
			logger::info("Error");
			}
		}
	return ret;
	}

// Calls functions registered with 'RegisterForExternalEvent'
// Mainly for Mantella actions
void sendMantellaEvent(std::monostate,
	std::string a_eventName,				// The name of the event being dispatched
	RE::Actor* target,						// Actor target of the action
	std::string stuff,						// Future use
	int handle) {							// JSON handle for action params

	const char* name;
	int targetID;
	if (target == nullptr) {
		name = "";
		targetID = 0;
		}
	else {
		name = target->GetDisplayFullName();
		targetID = target->GetFormID();
		}

	logger::info("SendMantellaEvent {} {} {} {}",
		a_eventName,
		name,
		stuff,
		handle);

	//Papyrus::detail::SendPapyrusExternalEvent(a_eventName, target, stuff, handle);
	// Passing Actor references doesn't always work,
	// So pass refID instead.
	Papyrus::detail::SendPapyrusExternalEvent(a_eventName, targetID, stuff, handle);
	}

void ClearCacheInternal(RE::BSAudioManager*) {									// Doesn't seem to do anything?
	RE::BSAudioManager* AudioMan = RE::BSAudioManager::GetSingleton();

	using func_t = decltype(ClearCacheInternal);
	static REL::Relocation<func_t> func{ REL::ID(114073) };
	func(AudioMan);
	}

void ClearCache(std::monostate) {
	ClearCacheInternal(nullptr);
	}

void UncacheAll() {
	using func_t = decltype(UncacheAll);
	static REL::Relocation<func_t> func{ REL::ID(40925) };
	func();
	}

// Override the file name used for wav and lip files
// We can use any name instead of the Topic FormID 

void SetOverrideFileName_internal(RE::TESTopicInfo* Info, char* name, uint64_t, uint64_t) {
	using func_t = decltype(SetOverrideFileName_internal);
	static REL::Relocation<func_t> func{ REL::VariantOffset(0x0621710, 0, 0x60C060) };	// Pre-NG, NG, VR

	logger::info("Info: {:p} Name; {:p}", (void*)Info, (void*)name);
	func(Info, name, 0, 0);
	}

void SetOverrideFileName(std::monostate, RE::TESTopic* this_p, std::string name) {
	RE::TESTopicInfo* tpInfo = this_p->topicInfos[0];

	UncacheAll();

	if (name != "00001ED2") {
		CopyFuz(name);
		}

	logger::info("SetOverrideFileName Info: {:p} name: {}", (void*)this_p, name.data());
	SetOverrideFileName_internal(tpInfo, name.data(), 0, 0);
	}

std::string StringRemoveWhiteSpace(std::monostate, std::string str) {
	if (str.length() == 0) return str;

	auto start = str.begin();
	while (start != str.end() && std::isspace(*start)) {
		start++;
		}

	auto end = str.end();
	do {
		end--;
		} while (std::distance(start, end) > 0 && std::isspace(*end));

	return std::string(start, end + 1);
	}

std::vector<std::string> split_string(std::monostate, std::string text, int width) {
	std::vector<std::string> ret;
	while (text.size() > width) {
		int end = text.rfind(' ', width);
		ret.push_back(text.substr(0, end));
		logger::info("Text: {}", text.substr(0, end));
		text = text.substr(end + 1);
		}
	logger::info("Text: {}", text);
	ret.push_back(text);
	return ret;
	}


void TakeScreenShot_internal(char* filename, int filetype, int sstype) {
	using func_t = decltype(TakeScreenShot_internal);
	static REL::Relocation<func_t> func{ REL::RelocationID(919230, 2229158) };

	func(filename, filetype, sstype);

	}

void TakeScreenShot(std::monostate, std::string filename, int filetype, int sstype) {
	TakeScreenShot_internal(filename.data(), filetype, sstype);
	}

bool RegisterInfoPatcherFunctions(std::string moduleName, RE::BSScript::IVirtualMachine* a_VM) {
	a_VM->BindNativeMethod(moduleName, "PatchTopicInfo", PatchTopicInfo, true);
	a_VM->BindNativeMethod(moduleName, "ClearCache", ClearCache, true);
	a_VM->BindNativeMethod(moduleName, "SetOverrideFileName", SetOverrideFileName, true);
	a_VM->BindNativeMethod(moduleName, "StringRemoveWhiteSpace", StringRemoveWhiteSpace, true);
	a_VM->BindNativeMethod(moduleName, "TakeScreenShot", TakeScreenShot, true);
	a_VM->BindNativeMethod(moduleName, "isMenuModeActive", isMenuModeActive, true);
	a_VM->BindNativeMethod(moduleName, "saveFloat", saveFloat, true);
	a_VM->BindNativeMethod(moduleName, "saveInt", saveInt, true);
	a_VM->BindNativeMethod(moduleName, "restoreFloat", restoreFloat, true);
	a_VM->BindNativeMethod(moduleName, "restoreInt", restoreInt, true);
	a_VM->BindNativeMethod(moduleName, "GetLastCrossHairActorID", GetLastCrossHairActorID, true);
	a_VM->BindNativeMethod(moduleName, "GetLastActorCoords", GetLastActorCoords, true);
	//a_VM->BindNativeMethod(moduleName, "GetLastCrossHairRef", GetLastCrosshairRef, true);
	//a_VM->BindNativeMethod(moduleName, "GetActorName", GetActorName, true);
	a_VM->BindNativeMethod(moduleName, "SendMantellaEvent", sendMantellaEvent, true);
	a_VM->BindNativeMethod(moduleName, "split_string", split_string, false);

	return true;
	}
