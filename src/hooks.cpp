#include <PCH.h>
#include "DetourXS/detourxs.h"
#include "detours/detours.h"


static REL::VariantOffset  TESFurniture_Activate(0x315610, 0, 0x2fb550);
DetourXS TESFurnitureActivateDetour;
static void* OrgAddr;

bool TESFurnitureActivateHook(RE::TESFurniture* Furn, RE::TESObjectREFR *Bench, RE::Actor *Act) {
	using func_t = decltype(TESFurnitureActivateHook);
	void * orgCode = TESFurnitureActivateDetour.GetTrampoline();
	func_t *funk = (func_t*)orgCode;
	
	logger::info("TESFurniture::Activate!");
	logger::info("Org: {:p}", orgCode);
	logger::info("Furn: {:p} {}", (void*) Furn, Furn->GetFullName());
	logger::info("Actor {:p} {}", (void*) Act, Act->GetDisplayFullName());
	logger::info("Bench {:p} {}", (void*) Bench, Bench->GetDisplayFullName());

	return funk(Furn, Bench, Act);
	}

static REL::ID ShowCraftingMenuID(877836);
DetourXS ShowCraftingMenuDetour;
static void* ShowCraftingMenuOrg;

void ShowCraftingMenuHook(RE::TESObjectREFR* bench) {
	using func_t = decltype(ShowCraftingMenuHook);
	void* orgCode = ShowCraftingMenuDetour.GetTrampoline();
	func_t* func = (func_t*)orgCode;

	if (REL::Module::IsVR()) {
	logger::info("ShowCraftingMenu bench={:p}", (void*) bench);
		func(bench);
		}
	}

static REL::Offset HUDQuickContainerID(0x0a932a0);
DetourXS HUDQuickContainerDetour;
static void* HUDQuickContainerOrg;

void HUDQuickContainerHook(RE::TESObjectREFR* bench) {
	using func_t = decltype(HUDQuickContainerHook);
	void* orgCode = HUDQuickContainerDetour.GetTrampoline();
	func_t* func = (func_t*)orgCode;
	logger::info("HUDQuickContainer");

	func(bench);
	}

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
	static bool IsActorInConversation(RE::Actor* act);
	};
}

void SubtitleManagerHideSubtitleHook(void* ptr, RE::TESObjectREFR *obj) {
	using func_t = decltype(SubtitleManagerHideSubtitleHook);
	//void* orgCode = SubtitleManagerHideSubtitleDetour.GetTrampoline();
	func_t* func = (func_t*)SubtitleManagerHideSubtitleOrg;
	bool actorInConversation = Hooks::MantellaDialogueTracker::IsActorInConversation((RE::Actor*) obj);
	static RE::TESObjectREFR* prevObj = nullptr;

	if (actorInConversation  && obj != prevObj) {
		const char* name = obj == nullptr ? "(null)" : obj->GetDisplayFullName();
		logger::info("Hide Subtitle {}", name);
		prevObj = obj;
		}

	func(ptr, obj);
	}


// Fix for ObjectReference.Activate not working in VR for workbenches.
// Vanilla code currently works only for P.A. stations.
// Fixes DankRafft's ECO

void patch_FurnitureActivate() {
	if (REL::Module::IsVR()) {
		uint8_t patch_bytes[2] = { 0, 0x74 };
		logger::info("Patching for VR");

		/* Change from:	CMP byte ptr[RAX + 0x178], 0x8			// TESFurniture.wbData  (8 == Power Armor)
		*				JNZ LAB_1402fb96f
		* to:			CMP byte ptr [RAX + 0x178], 0			// 0 = none
		*				JZ  LAB_1402fb96f
		*/

		REL::safe_write(TESFurniture_Activate.address() + 0x3ed, patch_bytes, sizeof(patch_bytes));
		}
	}

void initDetours() {
	logger::info("Init detours");

	//OrgAddr = (void*)TESFurniture_Activate.address();
	//TESFurnitureActivateDetour.Create(OrgAddr, TESFurnitureActivateHook);

	//ShowCraftingMenuOrg = (void*)ShowCraftingMenuID.address();
	//ShowCraftingMenuDetour.Create(ShowCraftingMenuOrg, ShowCraftingMenuHook);
	//HUDQuickContainerOrg = (void*)HUDQuickContainerID.address();
	//HUDQuickContainerDetour.Create(HUDQuickContainerOrg, HUDQuickContainerHook);
	//if (REL::Module::IsF4) {
	//	logger::info("Install Subtitle hooks");
	//	SubtitleManagerHideSubtitleOrg = (void*)SubtitleManagerHideSubtitleOffset.address();
	//	DetourTransactionBegin();
	//	DetourUpdateThread(GetCurrentThread());
	//	DetourAttach(&SubtitleManagerHideSubtitleOrg, SubtitleManagerHideSubtitleHook);
	//	DetourTransactionCommit();

	//	//SubtitleManagerHideSubtitleDetour.Create(SubtitleManagerHideSubtitleOrg, SubtitleManagerHideSubtitleHook);
	//	SubtitleManagerKillSubtitleOrg = (void*)SubtitleManagerKillSubtitleOffset.address();
	//	SubtitleManagerKillSubtitleDetour.Create(SubtitleManagerKillSubtitleOrg, SubtitleManagerKillSubtitleHook);
	//	}
	//SubtitleManagerShowSubtitleOrg = (void*)SubtitleManagerShowSubtitleOffset.address();
	//SubtitleManagerShowSubtitleDetour.Create(SubtitleManagerShowSubtitleOrg, SubtitleManagerShowSubtitleHook);
	//patch_FurnitureActivate();
	}