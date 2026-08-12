#include <PCH.h>
#include "DetourXS/detourxs.h"
#include "detours/detours.h"



static REL::ID SubtitleManagerShowSubtitleOffset(875508);
DetourXS SubtitleManagerShowSubtitleDetour;
static void* SubtitleManagerShowSubtitleOrg;

void SubtitleManagerShowSubtitleHook(void *ptr, RE::Actor *act, RE::BSFixedStringCS *parm3, RE::TESTopicInfo *topic, bool parm5) {
	using func_t = decltype(SubtitleManagerShowSubtitleHook);
	void* orgCode = SubtitleManagerShowSubtitleDetour.GetTrampoline();
	func_t* func = (func_t*)orgCode;

	const char* name = act->GetDisplayFullName();
	const char* info = topic->responses.head->responseText.c_str();

	logger::info("Subtitle: {} {}:{}", name, parm5, info);

	func(ptr, act,parm3, topic, parm5);
	}

static REL::Offset SubtitleManagerKillSubtitleOffset(0x12b3310);
DetourXS SubtitleManagerKillSubtitleDetour;
static void* SubtitleManagerKillSubtitleOrg;

void SubtitleManagerKillSubtitleHook(void* ptr) {
	using func_t = decltype(SubtitleManagerKillSubtitleHook);
	void* orgCode = SubtitleManagerKillSubtitleDetour.GetTrampoline();
	func_t* func = (func_t*)orgCode;

	logger::info("Kill Subtitles");

	func(ptr);
	}


static REL::Offset SubtitleManagerHideSubtitleOffset(0x12b3220);
//DetourXS SubtitleManagerHideSubtitleDetour;
static void* SubtitleManagerHideSubtitleOrg;

namespace Hooks {
struct MantellaDialogueTracker {
	static bool IsActorInConversation(RE::TESObjectREFR* act);
	};
}

void SubtitleManagerHideSubtitleHook(void* ptr, RE::TESObjectREFR *obj) {
	using func_t = decltype(SubtitleManagerHideSubtitleHook);
	//void* orgCode = SubtitleManagerHideSubtitleDetour.GetTrampoline();
	func_t* func = (func_t*)SubtitleManagerHideSubtitleOrg;
	bool actorInConversation = Hooks::MantellaDialogueTracker::IsActorInConversation(obj);
	static RE::TESObjectREFR* prevObj = nullptr;

	if (actorInConversation  && obj != prevObj) {
		const char* name = obj == nullptr ? "(null)" : obj->GetDisplayFullName();
		logger::info("Hide Subtitle {}", name);
		prevObj = obj;
		}

	func(ptr, obj);
	}


void initDetours() {
	logger::info("Init detours");

	if (REL::Module::IsF4) {
		//logger::info("Install Subtitle hooks");
		//SubtitleManagerHideSubtitleOrg = (void*)SubtitleManagerHideSubtitleOffset.address();
		//DetourTransactionBegin();
		//DetourUpdateThread(GetCurrentThread());
		//DetourAttach(&SubtitleManagerHideSubtitleOrg, SubtitleManagerHideSubtitleHook);
		//DetourTransactionCommit();

		//SubtitleManagerKillSubtitleOrg = (void*)SubtitleManagerKillSubtitleOffset.address();
		//SubtitleManagerKillSubtitleDetour.Create(SubtitleManagerKillSubtitleOrg, SubtitleManagerKillSubtitleHook);
		}
	}