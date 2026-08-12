#include <chrono>   // For std::chrono
#include <cstdint>  // For fixed-width integer types
#include <ctime>    // For std::strftime
#include <filesystem> // For std::filesystem
#include <fstream>  // For std::ofstream
#include <map>      // For std::map
#include <set>      // For std::set
#include <string>   // For std::string
#include <vector>   // For std::vector

//#include "MantellaDialogueIniConfig.h"
#include "MantellaPapyrusInterface.h"
#include "PCH.h"
#include <nlohmann\json.hpp>  // Include nlohmann/json library
#include "DetourXS/detourxs.h"
#include <KnownFolders.h>
#include <ShlObj.h>

extern void initDetours();

using json = nlohmann::json;

class SubtitleManager;

// Convert DialogueType to string (for debugging)
[[nodiscard]] const char* DialogueTypeToString(RE::DIALOGUE_TYPE a_type) noexcept {
	switch (a_type) {
		case RE::DIALOGUE_TYPE::kPlayerDialogue:    return "PlayerDialogue";
		case RE::DIALOGUE_TYPE::kCommandDialogue:   return "CommandDialogue";
		case RE::DIALOGUE_TYPE::kSceneDialogue:     return "SceneDialogue";
		case RE::DIALOGUE_TYPE::kCombat:            return "Combat";
		case RE::DIALOGUE_TYPE::kFavors:            return "Favors";
		case RE::DIALOGUE_TYPE::kDetection:         return "Detection";
		case RE::DIALOGUE_TYPE::kService:           return "Service";
		case RE::DIALOGUE_TYPE::kMiscellaneous:     return "Miscellaneous";
		default:                                  return "UnknownDialogueType";
		}
	}

// Convert DialogueSubtype to string (for debugging)
[[nodiscard]] const char* DialogueSubtypeToString(RE::DIALOGUE_SUBTYPE a_subtype) noexcept {
	switch (a_subtype) {
		// player dialogue
		case RE::DIALOGUE_SUBTYPE::kPlayerDialogue_Custom:        return "Player_Custom";
		case RE::DIALOGUE_SUBTYPE::kPlayerDialogue_Force_Greet:   return "Player_Force_Greet";
		case RE::DIALOGUE_SUBTYPE::kPlayerDialogue_Player_Rumor:  return "Player_Player_Rumor";
			// combined values (both share 0x3)
		case RE::DIALOGUE_SUBTYPE::kPlayerDialogue_End:           return "End/Command_Custom";

			// command dialogue
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Call:         return "Command_Call";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Follow:       return "Command_Follow";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Move:         return "Command_Move";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Attack:       return "Command_Attack";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Inspect:      return "Command_Inspect";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Retrieve:     return "Command_Retrieve";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Stay:         return "Command_Stay";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Release:      return "Command_Release";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_ShowRelationships: return "Command_ShowRelationships";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Reject:       return "Command_Reject";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Heal:         return "Command_Heal";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Assign:       return "Command_Assign";
		case RE::DIALOGUE_SUBTYPE::kCommandDialogue_Enter:        return "Command_Enter";

			// scene / favors / detection / service
		case RE::DIALOGUE_SUBTYPE::kSceneDialogue_Custom:         return "Scene_Custom";
		case RE::DIALOGUE_SUBTYPE::kFavor_Show:                   return "Favor_Show";
		case RE::DIALOGUE_SUBTYPE::kFavor_Agree:                  return "Favor_Agree";
		case RE::DIALOGUE_SUBTYPE::kFavor_Refuse:                 return "Favor_Refuse";
		case RE::DIALOGUE_SUBTYPE::kFavor_Exit_State:             return "Favor_Exit_State";
		case RE::DIALOGUE_SUBTYPE::kFavor_Morality_Refuse:        return "Favor_Morality_Refuse";
		case RE::DIALOGUE_SUBTYPE::kFavor_Trade:                  return "Favor_Trade";
		case RE::DIALOGUE_SUBTYPE::kFavor_Pathing_Reject:         return "Favor_Pathing_Reject";

			// combat (many)
		case RE::DIALOGUE_SUBTYPE::kCombat_Attack:                return "Combat_Attack";
		case RE::DIALOGUE_SUBTYPE::kCombat_Power_Attack:          return "Combat_Power_Attack";
		case RE::DIALOGUE_SUBTYPE::kCombat_Bash:                  return "Combat_Bash";
		case RE::DIALOGUE_SUBTYPE::kCombat_Hit:                   return "Combat_Hit";
		case RE::DIALOGUE_SUBTYPE::kCombat_Flee:                  return "Combat_Flee";
		case RE::DIALOGUE_SUBTYPE::kCombat_Bleed_Out:             return "Combat_Bleed_Out";
		case RE::DIALOGUE_SUBTYPE::kCombat_Avoid_Threat:          return "Combat_Avoid_Threat";
		case RE::DIALOGUE_SUBTYPE::kCombat_Death:                 return "Combat_Death";
		case RE::DIALOGUE_SUBTYPE::kCombat_Block:                 return "Combat_Block";
		case RE::DIALOGUE_SUBTYPE::kCombat_Taunt:                 return "Combat_Taunt";
		case RE::DIALOGUE_SUBTYPE::kCombat_Throw_Grenade:         return "Combat_Throw_Grenade";
		case RE::DIALOGUE_SUBTYPE::kCombat_Ally_Killed:           return "Combat_Ally_Killed";
		case RE::DIALOGUE_SUBTYPE::kCombat_Order_Fallback:        return "Combat_Order_Fallback";
		case RE::DIALOGUE_SUBTYPE::kCombat_Order_Advance:         return "Combat_Order_Advance";
		case RE::DIALOGUE_SUBTYPE::kCombat_Order_Flank:           return "Combat_Order_Flank";
		case RE::DIALOGUE_SUBTYPE::kCombat_Order_Take_Cover:      return "Combat_Order_Take_Cover";
		case RE::DIALOGUE_SUBTYPE::kCombat_Fallback:              return "Combat_Fallback";
		case RE::DIALOGUE_SUBTYPE::kCombat_Request_Support:       return "Combat_Request_Support";
		case RE::DIALOGUE_SUBTYPE::kCombat_Provide_Support:       return "Combat_Provide_Support";
		case RE::DIALOGUE_SUBTYPE::kCombat_Crippled_Limb:         return "Combat_Crippled_Limb";
		case RE::DIALOGUE_SUBTYPE::kCombat_Paired_Attack:         return "Combat_Paired_Attack";
		case RE::DIALOGUE_SUBTYPE::kCombat_Steal:                 return "Combat_Steal";
		case RE::DIALOGUE_SUBTYPE::kCombat_Yield:                 return "Combat_Yield";
		case RE::DIALOGUE_SUBTYPE::kCombat_Accept_Yield:          return "Combat_Accept_Yield";
		case RE::DIALOGUE_SUBTYPE::kCombat_Pickpocket:            return "Combat_Pickpocket";
		case RE::DIALOGUE_SUBTYPE::kCombat_Assault:               return "Combat_Assault";
		case RE::DIALOGUE_SUBTYPE::kCombat_Murder:                return "Combat_Murder";
		case RE::DIALOGUE_SUBTYPE::kCombat_Assault_NC:            return "Combat_Assault_NC";
		case RE::DIALOGUE_SUBTYPE::kCombat_Murder_NC:             return "Combat_Murder_NC";
		case RE::DIALOGUE_SUBTYPE::kCombat_Pickpocket_NC:         return "Combat_Pickpocket_NC";
		case RE::DIALOGUE_SUBTYPE::kCombat_Steal_NC:              return "Combat_Steal_NC";
		case RE::DIALOGUE_SUBTYPE::kCombat_Trespass_NC:           return "Combat_Trespass_NC";
		case RE::DIALOGUE_SUBTYPE::kCombat_Guard_Trespass:        return "Combat_Guard_Trespass";
		case RE::DIALOGUE_SUBTYPE::kCombat_Unused:                return "Combat_Unused";

			// voice power start/end (duplicates grouped)
		case RE::DIALOGUE_SUBTYPE::kCombat_Voice_Power_Start:     return "Combat_Voice_Power_Start";
		case RE::DIALOGUE_SUBTYPE::kCombat_Voice_Power_Start_Long:  return "Combat_Voice_Power_Start_Long";
		case RE::DIALOGUE_SUBTYPE::kCombat_Voice_Power_End_Short:
		case RE::DIALOGUE_SUBTYPE::kCombat_Voice_Power_End:        return "Combat_Voice_Power_End";

			// detection
		case RE::DIALOGUE_SUBTYPE::kDetection_Alert_Idle:          return "Detection_Alert_Idle";
		case RE::DIALOGUE_SUBTYPE::kDetection_Lost_Idle:           return "Detection_Lost_Idle";
		case RE::DIALOGUE_SUBTYPE::kDetection_Normal_To_Alert:     return "Detection_Normal_To_Alert";
		case RE::DIALOGUE_SUBTYPE::kDetection_Normal_To_Combat:    return "Detection_Normal_To_Combat";
		case RE::DIALOGUE_SUBTYPE::kDetection_Normal_To_Lost:      return "Detection_Normal_To_Lost";
		case RE::DIALOGUE_SUBTYPE::kDetection_Alert_To_Normal:     return "Detection_Alert_To_Normal";
		case RE::DIALOGUE_SUBTYPE::kDetection_Alert_To_Combat:     return "Detection_Alert_To_Combat";
		case RE::DIALOGUE_SUBTYPE::kDetection_Combat_To_Normal:    return "Detection_Combat_To_Normal";
		case RE::DIALOGUE_SUBTYPE::kDetection_Combat_To_Lost:      return "Detection_Combat_To_Lost";
		case RE::DIALOGUE_SUBTYPE::kDetection_Lost_To_Normal:      return "Detection_Lost_To_Normal";
		case RE::DIALOGUE_SUBTYPE::kDetection_Lost_To_Combat:      return "Detection_Lost_To_Combat";
		case RE::DIALOGUE_SUBTYPE::kDetection_Detect_Friend_Die:   return "Detection_Detect_Friend_Die";

			// service
		case RE::DIALOGUE_SUBTYPE::kService_Refusal:               return "Service_Refusal";
		case RE::DIALOGUE_SUBTYPE::kService_Repair:                return "Service_Repair";
		case RE::DIALOGUE_SUBTYPE::kService_Travel:                return "Service_Travel";
		case RE::DIALOGUE_SUBTYPE::kService_Training:              return "Service_Training";
		case RE::DIALOGUE_SUBTYPE::kService_Barter_Exit:           return "Service_Barter_Exit";
		case RE::DIALOGUE_SUBTYPE::kService_Repair_Exit:           return "Service_Repair_Exit";
		case RE::DIALOGUE_SUBTYPE::kService_Recharge:              return "Service_Recharge";
		case RE::DIALOGUE_SUBTYPE::kService_Recharge_Exit:         return "Service_Recharge_Exit";
		case RE::DIALOGUE_SUBTYPE::kService_Training_Exit:         return "Service_Training_Exit";

			// miscellaneous (many)
		case RE::DIALOGUE_SUBTYPE::kMisc_Observe_Combat:           return "Misc_Observe_Combat";
		case RE::DIALOGUE_SUBTYPE::kMisc_Corpse:                   return "Misc_Corpse";
		case RE::DIALOGUE_SUBTYPE::kMisc_Time_To_Go:               return "Misc_Time_To_Go";
		case RE::DIALOGUE_SUBTYPE::kMisc_Goodbye:                  return "Misc_Goodbye";
		case RE::DIALOGUE_SUBTYPE::kMisc_Hello:                    return "Misc_Hello";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Swing_Melee_Weapon: return "Misc_Player_Action_Swing_Melee_Weapon";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Shoot_Bow:   return "Misc_Player_Action_Shoot_Bow";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Z_Key_Object:return "Misc_Player_Action_Z_Key_Object";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Jumping:     return "Misc_Player_Action_Jumping";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Knocking_Over_Objects: return "Misc_Player_Action_Knocking_Over_Objects";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Destroy_Object: return "Misc_Player_Action_Destroy_Object";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Stand_On_Furniture: return "Misc_Player_Action_Stand_On_Furniture";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Locked_Object: return "Misc_Player_Action_Locked_Object";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Pickpocket:  return "Misc_Player_Action_Pickpocket";
		case RE::DIALOGUE_SUBTYPE::kMisc_Pursue_Idle:              return "Misc_Pursue_Idle";
		case RE::DIALOGUE_SUBTYPE::kMisc_InfoData:                 return "Misc_InfoData";
		case RE::DIALOGUE_SUBTYPE::kMisc_SceneChoice:              return "Misc_SceneChoice";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Cast_Projectile_Spell: return "Misc_Player_Action_Cast_Projectile_Spell";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Cast_Self_Spell: return "Misc_Player_Action_Cast_Self_Spell";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Shout:      return "Misc_Player_Action_Shout";
		case RE::DIALOGUE_SUBTYPE::kMisc_Idle:                     return "Misc_Idle";
		case RE::DIALOGUE_SUBTYPE::kMisc_Breath_Sprint_Sound:      return "Misc_Breath_Sprint_Sound";
		case RE::DIALOGUE_SUBTYPE::kMisc_Breath_Enter_Bow_Zoom_Sound:return "Misc_Breath_Enter_Bow_Zoom_Sound";
		case RE::DIALOGUE_SUBTYPE::kMisc_Breath_Exit_Bow_Zoom_Sound:return "Misc_Breath_Exit_Bow_Zoom_Sound";
		case RE::DIALOGUE_SUBTYPE::kMisc_Actor_Collide_With_Actor: return "Misc_Actor_Collide_With_Actor";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Iron_Sights:       return "Misc_Player_Iron_Sights";
		case RE::DIALOGUE_SUBTYPE::kMisc_Out_Off_Breath_Sound:     return "Misc_Out_Off_Breath_Sound";
		case RE::DIALOGUE_SUBTYPE::kMisc_Combat_Grunt:              return "Misc_Combat_Grunt";
		case RE::DIALOGUE_SUBTYPE::kMisc_Breath_Leave_Water:       return "Misc_Breath_Leave_Water";
		case RE::DIALOGUE_SUBTYPE::kMisc_Impatient_Positive:       return "Misc_Impatient_Positive";
		case RE::DIALOGUE_SUBTYPE::kMisc_Impatient_Negative:       return "Misc_Impatient_Negative";
		case RE::DIALOGUE_SUBTYPE::kMisc_Impatient_Neutral:        return "Misc_Impatient_Neutral";
		case RE::DIALOGUE_SUBTYPE::kMisc_Impatient_Question:       return "Misc_Impatient_Question";
		case RE::DIALOGUE_SUBTYPE::kMisc_Waiting_Player_Input:     return "Misc_Waiting_Player_Input";
		case RE::DIALOGUE_SUBTYPE::kMisc_Greeting:                 return "Misc_Greeting";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Activate_Door:     return "Misc_Player_Activate_Door";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Activate_Terminals: return "Misc_Player_Activate_Terminals";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Activate_Furniture: return "Misc_Player_Activate_Furniture";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Activate_Activators: return "Misc_Player_Activate_Activators";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Activate_Container:return "Misc_Player_Activate_Container";
		case RE::DIALOGUE_SUBTYPE::kMisc_Player_Aquire_Featured_Item:return "Misc_Player_Aquire_Featured_Item";

			// final fallback
		default:                                              return "UnknownDialogueSubtype";
		}
	}


#ifdef _F4AE
#define LOG_debug(...) REX::DEBUG(__VA_ARGS__)
#define LOG_info(...) REX::INFO(__VA_ARGS__)
#define LOG_warn(...) REX::WARN(__VA_ARGS__)
#define LOG_error(...) REX::ERROR(__VA_ARGS__)
#else
#define LOG_debug(...) logger::debug(__VA_ARGS__)
#define LOG_info(...) logger::info(__VA_ARGS__)
#define LOG_warn(...) logger::warn(__VA_ARGS__)
#define LOG_error(...) logger::error(__VA_ARGS__)
#endif


namespace Hooks {

static inline std::string dialogueEventsBuffer;

	// -------------------------------------------------------------------------
	// A small POD struct to store a single exchange: line spoken, actor name
	// and the in-game time at which it was recorded.
	// -------------------------------------------------------------------------
	struct DialogueLine {
		std::string Line;
		std::string Name;
		float gameTimeHours;
		};

	
	inline void to_json(json& j, const DialogueLine& line) {
		j = json{ {"Name", line.Name},
				 {"Response", line.Line},
				 {"gameTimeHours", line.gameTimeHours}
			};
		}

	inline void from_json(const json& j, DialogueLine& line) {
		j.at("Name").get_to(line.Name);
		j.at("Response").get_to(line.Line);
		j.at("gameTimeHours").get_to(line.gameTimeHours);
		}


	static bool IsEnabled() { return MantellaPapyrusInterface::GetMantellaEnableVanillaDialogueAwareness(); }

	// -------------------------------------------------------------------------
	// A simple helper to fetch current game time in hours.
	// -------------------------------------------------------------------------
	static float GetCurrentGameTimeHours() {
		auto calendar = RE::Calendar::GetSingleton();
		return calendar ? calendar->GetHoursPassed() : 0.0f;
	}

	// -------------------------------------------------------------------------
	// MantellaDialogueTracker:
	// - holds the participants list form
	// - tracks if there's an internal error
	// - stores unsent dialogue lines in a map keyed by Actor form ID
	// -------------------------------------------------------------------------
	struct MantellaDialogueTracker {
		static inline RE::BGSListForm* aParticipants;
		static inline bool DialogueTrackerHasError = false;

		// Key = actor form ID, value = list of dialogue lines not yet added to Mantella
		static inline std::map<RE::TESFormID, std::vector<DialogueLine>> s_dialogueHistory{};
		static inline float lastCapturedGameTimeHours = 0.0f; // To track the game time of the last captured line

		struct QuestPreCache {
			std::vector<DialogueLine> lines;
			bool playerSeen = false;
			bool NPCseen = false;
			std::set<RE::TESFormID> actorsInConversation;
			};
		static inline std::map<std::string, QuestPreCache> s_preCacheByQuest{};

		static void Setup() {
			auto dataHandler = RE::TESDataHandler::GetSingleton();
			if (!dataHandler) {
				LOG_error("!!! MantellaDialogueTracker::Setup: TESDataHandler is null!");
				DialogueTrackerHasError = true;
				return;
				}
			aParticipants = dataHandler->LookupForm<RE::BGSListForm>(0x04C57, "Mantella.esp");
			if (!aParticipants) {
				LOG_error("!!! MantellaDialogueTracker::Setup: aParticipants is null!");
				DialogueTrackerHasError = true;
				return;
				}
			}

		// ---------------------------------------------------------------------
		// Helper: check if the conversation is considered "running" by looking
		// at the participants list.
		// ---------------------------------------------------------------------
		static bool IsConversationRunning() {
			if (!aParticipants || DialogueTrackerHasError) return false;
			auto scriptAddedForms = aParticipants->scriptAddedTempForms;
			if (!scriptAddedForms) return false;
			return !scriptAddedForms->empty();
			}

		// ---------------------------------------------------------------------
		// Helper: checks if a given actor is listed in the current participants form list.
		// ---------------------------------------------------------------------
		static bool IsActorInConversation(RE::TESObjectREFR* a_actor) {
			if (!a_actor || !aParticipants || DialogueTrackerHasError) return false;
			auto formID = a_actor->GetFormID();
			auto scriptAddedForms = aParticipants->scriptAddedTempForms;
			if (!scriptAddedForms) return false;
			for (auto& actorInConvFormId : *scriptAddedForms)
				if (actorInConvFormId == formID) return true;
			return false;
			}

		// ---------------------------------------------------------------------
		// Called when a conversation starts.
		// ---------------------------------------------------------------------
		static void OnConversationStarted() {
			std::string buf;
			for (auto actorID : *aParticipants->scriptAddedTempForms) {
				RE::Actor* actor = RE::TESForm::GetFormByID<RE::Actor>(actorID);
				buf += (actor == nullptr) ? "<null>" : actor->GetDisplayFullName();
				buf += " ";
				}
			LOG_info("vanilla dialogue awareness is : {}", IsEnabled() ? "ON" : "OFF");
			LOG_info("Conversation started with participants: {}", buf);
			}

		// Sends the dialogue that was captured when not in a conversation to Mantella and removes it from the backlog
		static void SendAndDiscardCapturedDialogue(RE::Actor* a_actor) {
			static RE::Actor* player = RE::PlayerCharacter::GetSingleton();
			static std::string_view playerName = player->GetDisplayFullName();
			std::string concatenatedLines;
			std::string_view actorName = a_actor ? a_actor->GetDisplayFullName() : "Unknown Actor";
			RE::TESFormID NPCFormID = a_actor ? a_actor->GetFormID() : 0;
			bool foundPlayer = false;
			bool foundActor = false;
			int lineCount = 0;

			if (a_actor == player) return;

			auto it = s_dialogueHistory.find(NPCFormID);
			if (it == s_dialogueHistory.end()) return;

			for (DialogueLine& dl :  it->second) {
				if (dl.Name == playerName) {
					foundPlayer = true;
					}
				if (dl.Name == actorName) {
					foundActor = true;
					}
				concatenatedLines += dl.Name + ": " + dl.Line + ";\n ";
				lineCount++;
				}
			it->second.clear(); // Clear the dialogue lines for this actor after processing
			

			// If we found lines for the current actor and the player, send them to Mantella
			if (foundActor) {
				if (foundPlayer) {
					LOG_debug("{} had {} captured lines. Stored it in dialogueEventsBuffer", actorName, lineCount);
					// Remove the trailing space, if any
					if (!concatenatedLines.empty() && concatenatedLines.back() == ' ') concatenatedLines.pop_back();
					// Send a single Mantella event with the concatenated lines
					if (!concatenatedLines.empty()) {

						//MantellaPapyrusInterface::AddMantellaEvent(concatenatedLines.c_str());
						dialogueEventsBuffer.append(concatenatedLines);
						}
					}

				s_dialogueHistory.erase(NPCFormID); // Remove the conversation from history after sending
				}
			}

		// ---------------------------------------------------------------------
		// Called if a new participant joins conversation.
		// ---------------------------------------------------------------------
		static void OnNewParticipant(RE::Actor* a_newActor) {
			if (!a_newActor || DialogueTrackerHasError) return;
			std::string buf;
			for (auto actorID : *aParticipants->scriptAddedTempForms) {
				RE::Actor* actor = RE::TESForm::GetFormByID<RE::Actor>(actorID);
				buf += (actor == nullptr) ? "<null>" : actor->GetDisplayFullName();
				buf += " ";
				}
			LOG_info("{} added to conversation: {}", a_newActor->GetDisplayFullName(), buf);
			SendAndDiscardCapturedDialogue(a_newActor);
			}
		};


	static REL::ID SubtitleManagerShowSubtitleOffset(875508);
	DetourXS SubtitleManagerShowSubtitleDetour;
	static void* SubtitleManagerShowSubtitleOrg;

	enum QuestType {
			 qtNone,
			 qtMainQuest,
			 qtBrotherhoodOfSteel,
			 qtInstitute,
			 qtMinutemen,
			 qtRailroad,
			 qtMiscellaneous,
			 qtSideQuests,
			 qtDLC01,
			 qtDLC02,
			 qtDLC03,
			 qtDLC04,
			 qtDLC05,
			 qtDLC06,
			 qtDLC07
			};

	// Convert QuestType to string (for debugging)
	[[nodiscard]] const char* QuestTypeToString(uint8_t a_questType) noexcept {
		switch (a_questType) {
			case qtNone:                return "None";
			case qtMainQuest:           return "MainQuest";
			case qtBrotherhoodOfSteel:  return "BrotherhoodOfSteel";
			case qtInstitute:           return "Institute";
			case qtMinutemen:           return "Minutemen";
			case qtRailroad:            return "Railroad";
			case qtMiscellaneous:       return "Miscellaneous";
			case qtSideQuests:          return "SideQuests";
			case qtDLC01:               return "DLC01";
			case qtDLC02:               return "DLC02";
			case qtDLC03:               return "DLC03";
			case qtDLC04:               return "DLC04";
			case qtDLC05:               return "DLC05";
			case qtDLC06:               return "DLC06";
			case qtDLC07:               return "DLC07";
			default:                    return "UnknownQuestType";
			}
		}

	// -------------------------------------------------------------------------
	// ShowSubtitle hook:
	// -------------------------------------------------------------------------
	struct ShowSubtitle {
		inline static std::string s_lastPlayerTopicText{};
		static bool HasAlreadyProcessed(std::string_view a_topicText) {
			return a_topicText == s_lastPlayerTopicText;
			}
		static void UpdateLastPlayerTopicText(std::string_view a_topicText) {
			s_lastPlayerTopicText = std::string(a_topicText);
			}

		static void AddDialogueExchangeAsync(const DialogueLine& exchange) {
			dialogueEventsBuffer += exchange.Name + ": " + exchange.Line + ";\n";

			//MantellaPapyrusInterface::AddMantellaEvent(exchange.Name + ": " + exchange.Line);
		}

		// ---------------------------------------------------------------------
		// Appends a single row to DialogueFilter.csv in the F4SE logging folder.
		// Each line is prefixed with '*' if the dialogue was filtered out
		// (ShouldFilterDialogue returned true), or '-' otherwise.
		// Call InitDialogueFilterCsv() on game start / save load to open a new file.
		// ---------------------------------------------------------------------
		static inline std::filesystem::path s_csvPath{};

		static void InitDialogueFilterCsv() {
			wchar_t* knownBuffer{ nullptr };
			const auto knownResult = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, std::addressof(knownBuffer));
			std::unique_ptr<wchar_t[], decltype(&CoTaskMemFree)> knownPath(knownBuffer, CoTaskMemFree);
			if (!knownPath || knownResult != 0) {
				LOG_error("failed to get known folder path");
				return;
				}

			std::filesystem::path path = knownPath.get();
			path /= "My Games/Fallout4/F4SE/";


			auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm tm{};
#if defined(_WIN32)
			localtime_s(&tm, &now);
#else
			localtime_r(&now, &tm);
#endif
			char timestamp[32];
			std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);

			s_csvPath = path / (std::string("DialogueFilter_") + timestamp + ".csv");
			s_csvPath.make_preferred();

			std::ofstream csv;
			csv.open(s_csvPath, std::ios::out | std::ios::trunc);

			if (!csv.is_open()) {
				std::error_code err;
				LOG_error("DialogueFilter: {} : failed to create {}", err.message(),  s_csvPath.string());
				s_csvPath.clear();
				return;
				}
			csv << "Prefix,QuestName,QuestType,DialogueType,DialogueSubtype,speaker, target, Line\n";
			LOG_info("DialogueFilter: logging to {}", s_csvPath.string());

			// Rotate: keep only the 5 most recent DialogueFilter_*.csv files.
			constexpr int kMaxFiles = 5;
			std::vector<std::filesystem::path> existing;
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				auto& p = entry.path();
				if (p.extension() == ".csv" && p.filename().string().starts_with("DialogueFilter_"))
					existing.push_back(p);
				}
			std::sort(existing.begin(), existing.end()); // lexicographic = chronological for YYYYMMDD_HHMMSS names
			while (static_cast<int>(existing.size()) > kMaxFiles) {
				std::error_code ec;
				std::filesystem::remove(existing.front(), ec);
				if (ec)
					LOG_warn("DialogueFilter: failed to delete old log {}: {}", existing.front().string(), ec.message());
				existing.erase(existing.begin());
				}
			}

		static void LogDialogueFilterCsv(bool a_filtered, const std::string a_speakerName, const char * a_targetName, const char* a_questName, const char* a_questTypeStr,
				const char* a_typeStr, const char* a_subtypeStr, const std::string& a_line) {
			if (s_csvPath.empty()) return;
			//LOG_info("CSV path {}", s_csvPath.string());
			std::ofstream csv(s_csvPath, std::ios::out | std::ios::app);
			if (!csv.is_open()) return;
			//LOG_info("CSV Log: {} {} {} {} {} {} {}", a_filtered ? '*' : '-', a_speakerName, a_questName, a_questTypeStr, a_typeStr, a_subtypeStr, a_line);

			csv << (!a_filtered ? '*' : '-') << ','
				<< '"' << a_questName << '"' << ','
				<< '"' << a_questTypeStr << '"' << ','
				<< '"' << a_typeStr << '"' << ','
				<< '"' << a_subtypeStr << '"' << ','
				<< '"' << a_speakerName << '"' << ','
				<< '"' << a_targetName << '"' << ','
				<< '"' << a_line << '"' << '\n';
			//LOG_info("CSV Log: done");
			}
		/*
			Filter dialogue lines that should be saved and passed to Mantella.
			Include only:
			Dialogue type == kSceneDialogue
			Quest type    != qtNone

			Also excludes any dialogue generated by Mantella.
		*/

		static bool ShouldFilterDialogue(RE::TESObjectREFR *speaker, std::string line, RE::TESTopicInfo* topicInfo) {
			if (HasAlreadyProcessed(line)) return true;

			if (speaker == nullptr || topicInfo == nullptr) return true;
			const char* speakerName = speaker->GetDisplayFullName();
			if (speakerName == nullptr) speakerName = "<unknown>";

			RE::TESTopic* topic = topicInfo->parentTopic;
			if (topic == nullptr) return true;

			RE::TESQuest* quest = topic->ownerQuest;
			if (quest == nullptr) return true;

			RE::DIALOGUE_TYPE type = (RE::DIALOGUE_TYPE)topic->data.type;
			RE::DIALOGUE_SUBTYPE subtype = (RE::DIALOGUE_SUBTYPE)topic->data.subtype;
			uint8_t questtype = quest->data.questType;
			const char *questName = quest->formEditorID.c_str();
			const char *typeStr = DialogueTypeToString(type);
			const char *subtypeStr = DialogueSubtypeToString(subtype);
			const char *questTypeStr = QuestTypeToString(questtype);
			RE::Actor* playerRef = RE::PlayerCharacter::GetSingleton();
			RE::TESObjectREFR* dialogueTarget = nullptr;

			RE::Actor* speakerActor = speaker->As<RE::Actor>();
			const char* targetName = "<nobody>";

			if (speakerActor != nullptr) {
				dialogueTarget = speakerActor->dialogueItemTarget.get().get();
				targetName = dialogueTarget ? dialogueTarget->GetDisplayFullName() : "nobody";
				}
			else {
				LOG_info("Speaker is not an actor");				// Not an actor
				}


			if (type == RE::DIALOGUE_TYPE::kSceneDialogue
				|| (type == RE::DIALOGUE_TYPE::kMiscellaneous && subtype == RE::DIALOGUE_SUBTYPE::kMisc_Greeting)) {
				bool isPlayerSpeaking = speaker == playerRef;

				static RE::TESQuest* playerQuest;
				if (isPlayerSpeaking && playerQuest != quest) {
					LOG_info("Player quest {}", questName);
					playerQuest = quest;
					}

				bool ret = !isPlayerSpeaking && dialogueTarget != playerRef;

				LogDialogueFilterCsv(ret, speakerName, targetName, questName, questTypeStr, typeStr, subtypeStr, line.c_str());
				return ret;
				}

			if (type == RE::DIALOGUE_TYPE::kMiscellaneous
				&& subtype == RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Stand_On_Furniture) {
				return true;    // Mantella
				}
			LogDialogueFilterCsv(true, speakerName, targetName, questName, questTypeStr, typeStr, subtypeStr, line.c_str());
			return true;
			}

		// The hook function
		static void SubtitleManagerShowSubtitleHook(
				SubtitleManager* a_this, RE::TESObjectREFR* a_speaker, RE::BSFixedStringCS *a_subtitle,
				RE::TESTopicInfo *topicinfo, bool a_alwaysDisplay) {

			static bool s_firstCall = true;
			using MDT = MantellaDialogueTracker;
			using func_t = decltype(SubtitleManagerShowSubtitleHook);
			void* orgCode = SubtitleManagerShowSubtitleDetour.GetTrampoline();
			func_t* func = (func_t*)orgCode;

			if (s_firstCall) {
				LOG_info("Subtitle Hook called.");
				LOG_info("vanilla dialogue awareness is : {}", IsEnabled() ? "ON" : "OFF");
				s_firstCall = false;
				}

			//LOG_info("speaker is {:04X}: {:04X} {:p}", a_speaker->formID, (int)a_speaker->FORM_ID, (void *) derp );

			// Call original
			func(a_this, a_speaker, a_subtitle, topicinfo, a_alwaysDisplay);

			if (topicinfo == nullptr) {
				LOG_info("SubtitleManagerShowSubtitleHook: topicinfo is null");
				return;
				}

			RE::TESTopic* topic = topicinfo->parentTopic;
			if (topic == nullptr) {
				LOG_info("SubtitleManagerShowSubtitleHook: topic is null");
				return;
				}

			if (a_speaker == nullptr) {
				LOG_info("SubtitleManagerShowSubtitleHook: speaker is null");
				return;
				}

			// Query RTTI classname of the speaker for debugging
			const char* speakerRttiName = "<null>";
			//try {
			//	speakerRttiName = typeid(*a_speaker).name();
			//	}
			//catch (...) {
			//	speakerRttiName = "<rtti-error>";
			//	}
			
			RE::TESObjectREFR* speakerActor = a_speaker->As<RE::TESObjectREFR>();
			if (speakerActor == nullptr) {
				LOG_info("SubtitleManagerShowSubtitleHook: speaker is not an Actor, RTTI class: {}", speakerRttiName);
				return;
				}

			RE::TESFormID speakerFormID = a_speaker->formID;

			if (!IsEnabled() || MDT::DialogueTrackerHasError) {
				//LOG_debug(" -> Mantella dialogue awareness is disabled.");
				return;
				}


#if 0
			if (RE::PlayerCamera::GetSingleton()->QCameraEquals(RE::CameraState::kFree)) {
				RE::DIALOGUE_TYPE topic_type = (RE::DIALOGUE_TYPE)topic->data.type;
				RE::DIALOGUE_SUBTYPE topic_subtype = (RE::DIALOGUE_SUBTYPE)topic->data.subtype;
				const char* caller = nullptr;

				//const char* caller = nullptr;
				if (REL::Module::IsF4()) {
					void* returnAddress = _ReturnAddress();
					uint32_t offset = (uint32_t)returnAddress & 0xffff;
					if (offset == 0xe9ad) caller = "VoiceSoundFunction";
					if (offset == 0xf086) caller = "SayForceGoodbyeGreetingToPlayer";
					if (offset == 0x2c27) caller = "VoiceSoundFunctionEx";
					if (offset == 0x3aca) caller = "GetFullLODRef";
					if (offset == 0x9c81) caller = "UpdateVoiceTimer";

					if (caller == nullptr) {
						LOG_info("Unexpected {:p} {} {}", returnAddress,
							DialogueTypeToString(topic_type), DialogueSubtypeToString(topic_subtype));
						}
					else {
						LOG_info("{} [{}] {}: {}", caller, a_alwaysDisplay ? "Force" : "Normal", a_speaker->GetDisplayFullName(), a_subtitle->c_str());
						//if (offset != 0x3aca || topic_type != RE::DIALOGUE_TYPE::kMiscellaneous
						//	|| topic_subtype != RE::DIALOGUE_SUBTYPE::kMisc_Player_Action_Stand_On_Furniture) {
						//	LOG_info("{} {}:{}", caller,
						//		DialogueTypeToString(topic_type), DialogueSubtypeToString(topic_subtype));
						//	}
						}
					}
				else {
					caller = "";
					}
				}
#endif

			//LOG_info("Speaker ID {:04X}", speakerFormID);

			// Safe logging: avoid dereferencing pointers that may be null
			const char* subtitleStr = a_subtitle && a_subtitle->c_str() ? a_subtitle->c_str() : "<null>";
			const char* speakerName = "<null>";
			const char* tmp = a_speaker->GetDisplayFullName();
			speakerName = (tmp && tmp[0] != '\0') ? tmp : "<unnamed>";

			bool conversationRunning = MDT::IsConversationRunning();
			bool actorInConversation = MDT::IsActorInConversation(a_speaker);
			bool isPlayerSpeaking = a_speaker == RE::PlayerCharacter::GetSingleton();

			auto exchange = DialogueLine();
			exchange.Line = subtitleStr;
			exchange.Name = speakerName;
			exchange.gameTimeHours = GetCurrentGameTimeHours();

			RE::TESQuest* quest = topic->ownerQuest;
			if (quest == nullptr) return;
			const char* questName = quest->formEditorID.c_str();

			if (ShouldFilterDialogue(a_speaker, exchange.Line, topicinfo)) return;
			bool newConvo = exchange.gameTimeHours - MDT::lastCapturedGameTimeHours > 2.0f;
			MDT::lastCapturedGameTimeHours = exchange.gameTimeHours;

			//LOG_info("{}: {}", exchange.Name, exchange.Line);

			if (!conversationRunning) {
				// Vanilla dialogue occurring outside a Mantella conversation.
				// Each quest has its own pre-cache bucket. A bucket is only promoted to
				// dialogue history once we have seen both a player line and an NPC line
				// for that quest; otherwise it is discarded.

				if (newConvo) {
					// Time gap detected — flush all quest buckets.
					for (auto& [qName, qpc] : MDT::s_preCacheByQuest) {
						if (qpc.playerSeen && qpc.NPCseen) {
							for (auto NPCFormID : qpc.actorsInConversation) {
								auto& lines = MDT::s_dialogueHistory[NPCFormID];
								lines.append_range(qpc.lines);
								LOG_info("Flushed {} cached lines from quest '{}' to history of NPC {:08X}",
									qpc.lines.size(), qName, NPCFormID);
								}
							}
						else {
							LOG_info("Discarding {} cached lines from quest '{}' (no player/NPC pair)",
								qpc.lines.size(), qName);
							}
						}
					MDT::s_preCacheByQuest.clear();
					}

				// Route current line into its quest bucket.
				auto& qpc = MDT::s_preCacheByQuest[questName];

				if (isPlayerSpeaking) {
					qpc.playerSeen = true;
					s_lastPlayerTopicText = exchange.Line;
					}
				else {
					qpc.NPCseen = true;
					qpc.actorsInConversation.insert(a_speaker->GetFormID());
					}

				if (qpc.playerSeen && qpc.NPCseen) {
					// Flush any buffered lines for this quest now that we have both sides.
					if (!qpc.lines.empty()) {
						for (auto NPCFormID : qpc.actorsInConversation) {
							auto& lines = MDT::s_dialogueHistory[NPCFormID];
							lines.append_range(qpc.lines);
							LOG_info("Flushed {} buffered lines from quest '{}' to history of NPC {:08X}",
								qpc.lines.size(), questName, NPCFormID);
							}
						qpc.lines.clear();
						}
					for (auto NPCFormID : qpc.actorsInConversation) {
						MDT::s_dialogueHistory[NPCFormID].push_back(exchange);
						}
					}
				else {
					qpc.lines.push_back(exchange);
					}
				}
			else {      // Conversation is running
				 if (actorInConversation) {
					AddDialogueExchangeAsync(exchange);
					//LOG_info("  -> Sent dialogue to Mantella");
					 }
				 else {
					 //if (quest != playerQuest) {
						// LOG_info("Ignoring Speaker {}: {}", speakerName, exchange.Line);
						// return;
						// }

					 RE::TESFormID NPCFormID = a_speaker->formID;
					 AddDialogueExchangeAsync(exchange);
					 LOG_info("  -> Actor {} not in conversation, sent dialogue to Mantella anyways", speakerName);
					 if (!MDT::s_dialogueHistory.contains(NPCFormID)) {
						 // If we haven't seen any lines from this NPC yet, initialize an empty conversation for them, to store the player's lines until the conversation starts
						 MDT::s_dialogueHistory[NPCFormID] = {};
						 }
					 MDT::s_dialogueHistory[NPCFormID].push_back(exchange);
					 }
				}
			UpdateLastPlayerTopicText(exchange.Line);
			}
	};

}  // namespace Hooks


#pragma region Serialization
// -----------------------------------------------------------------------------
// JSON Serialization and Deserialization Functions
// -----------------------------------------------------------------------------
constexpr size_t MAX_DIALOGUE_LINES = 20000;
std::string SerializeDialogueHistoryToJSON() {
	using MDT = Hooks::MantellaDialogueTracker;
	size_t totalDialogueLines = 0;
	json j;

	float kMaxAgeHours = MantellaPapyrusInterface::GetMantellaDialogueExpiration();
	kMaxAgeHours = std::max(kMaxAgeHours, 6.0f); // Ensure non-negative
	kMaxAgeHours = std::min(kMaxAgeHours, 240.0f); // Cap at 10 days

	const float currentGameTimeHours = Hooks::GetCurrentGameTimeHours();

	for (const auto& [formID, dialogueLines] : MDT::s_dialogueHistory) {
		totalDialogueLines += dialogueLines.size();
		if (totalDialogueLines > MAX_DIALOGUE_LINES) {
			LOG_warn("!!! Exceeded max dialogue lines threshold: Discarding all saved dialogue history.");
			return "{}";
			}
		}

	// Drop dialogue lines older than 10 in-game days, to avoid storing lines from a character that you might never talk with.
	for (const auto& [formID, dialogueLines] : MDT::s_dialogueHistory) {
		std::vector<Hooks::DialogueLine> recentLines;
		for (const auto& dl : dialogueLines) {
			if (currentGameTimeHours - dl.gameTimeHours <= kMaxAgeHours) {
				recentLines.push_back(dl);
				}
			else {
				RE::TESObjectREFR* actor = RE::TESForm::GetFormByID<RE::TESObjectREFR>(formID);
				std::string formIDstr = std::format("{:04X}", formID);
				std::string actorName = actor ? actor->GetDisplayFullName() : formIDstr;
				if (actorName == "Settler") {
					actorName = std::format("Settler {}", formIDstr);
					}
				LOG_debug("Dropping stale dialogue line from {} (age: {:.1f}h > {:.0f}h limit)",
					actorName, currentGameTimeHours - dl.gameTimeHours, kMaxAgeHours);
				}
			}
		if (!recentLines.empty())
			j[std::to_string(formID)] = recentLines;
		}

	// Serialize per-quest pre-cache buckets
	json preCacheJson = json::object();
	for (const auto& [qName, qpc] : MDT::s_preCacheByQuest) {
		preCacheJson[qName] = {
			{"lines", qpc.lines},
			{"playerSeen", qpc.playerSeen},
			{"NPCseen", qpc.NPCseen},
			{"actorsInConversation", std::vector<RE::TESFormID>(qpc.actorsInConversation.begin(), qpc.actorsInConversation.end())}
			};
		}
	j["__DialogueTrackerState"] = {{"preCacheByQuest", preCacheJson}};

	return j.dump();
	}

bool DeserializeDialogueHistoryFromJSON(const std::string& jsonString, const F4SE::SerializationInterface* a_intfc) {
	using MDT = Hooks::MantellaDialogueTracker;
	try {
		json j = json::parse(jsonString);
		MDT::s_dialogueHistory.clear();
		
		for (auto it = j.begin(); it != j.end(); ++it) {
			if (it.key() == "__DialogueTrackerState") continue;
			RE::TESFormID formID = static_cast<RE::TESFormID>(std::stoul(it.key()));

			std::optional<RE::TESFormID> newformID = a_intfc->ResolveFormID(formID);		// Mod index may have changed, so resolve the form ID to the current one
			if (newformID.has_value() && newformID.value() != formID) {
				LOG_info("Mod load: remapped form ID {:04X} -> {:04X}", formID, newformID.value());
				formID = newformID.value();
				}

			std::string formIDstr = std::format("{:04X}", formID);
			RE::Actor* form = RE::TESForm::GetFormByID<RE::Actor>(formID);
			std::string formName = form ? form->GetDisplayFullName() : formIDstr;
			if (formName == "Settler") {
				formName = std::format("Settler {}", formIDstr);
				}

			LOG_info("Deserializing dialogue history for form ID {} : {} dialogue lines", formName, it.value());
			std::vector<Hooks::DialogueLine> dialogueLines = it.value().get<std::vector<Hooks::DialogueLine>>();
			LOG_debug("{}: {} pending lines", formName, dialogueLines.size());
			MDT::s_dialogueHistory.emplace(formID, std::move(dialogueLines));
			}

		if (j.contains("__DialogueTrackerState")) {
			auto& state = j["__DialogueTrackerState"];
			MDT::s_preCacheByQuest.clear();
			if (state.contains("preCacheByQuest")) {
				for (auto it = state["preCacheByQuest"].begin(); it != state["preCacheByQuest"].end(); ++it) {
					Hooks::MantellaDialogueTracker::QuestPreCache qpc;
					if (it->contains("lines"))
						qpc.lines = it->at("lines").get<std::vector<Hooks::DialogueLine>>();
					qpc.playerSeen = it->value("playerSeen", false);
					qpc.NPCseen = it->value("NPCseen", false);
					auto tmpActors = it->value("actorsInConversation", std::vector<RE::TESFormID>{});
					qpc.actorsInConversation = { tmpActors.begin(), tmpActors.end() };
					MDT::s_preCacheByQuest.emplace(it.key(), std::move(qpc));
					}
				}
			}

		return true;
	} catch (const json::parse_error& e) {
		LOG_error("!!! Failed to parse dialogue history JSON: %s", e.what());
		return false;
	} catch (const std::exception& e) {
		LOG_error("!!! Exception during deserialization: %s", e.what());
		return false;
	}
}

constexpr std::uint32_t kSerializationID = 'MTDL';

void MySaveCallback(const F4SE::SerializationInterface* a_intfc) {
	LOG_info("MySaveCallback: Saving dialogue history to F4SE co-save...");

	try {
		std::string jsonString = SerializeDialogueHistoryToJSON();
		constexpr std::uint32_t recordType = 'HIST';
		constexpr std::uint32_t version = 1;
		if (!a_intfc->OpenRecord(recordType, version)) {
			LOG_error("!!! MySaveCallback: Failed to open 'HIST' record for serialization.");
			return;
		}
		std::uint32_t jsonLength = static_cast<std::uint32_t>(jsonString.size());
		if (!a_intfc->WriteRecordData(&jsonLength, sizeof(jsonLength))) {
			LOG_error("!!! MySaveCallback: Failed to write JSON string length.");
			return;
		}
		if (!a_intfc->WriteRecordData(jsonString.c_str(), jsonLength)) {
			LOG_error("!!! MySaveCallback: Failed to write JSON string data.");
			return;
		}
		LOG_info("MySaveCallback: Serialized {}k dialogue history to F4SE co-save.", jsonLength / 1024);
	} catch (const std::exception& e) {
		LOG_error(" !!! MySaveCallback: Exception during serialization: %s", e.what());
	}

}

void MyLoadCallback(const F4SE::SerializationInterface* a_intfc) {
	LOG_info("MyLoadCallback: Loading dialogue history from F4SE co-save...");
	Hooks::ShowSubtitle::InitDialogueFilterCsv();

	try {
		std::uint32_t type, version, length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			LOG_info("MyLoadCallback: Found record type {:c}{:c}{:c}{:c}', version {}, length {}",
				(type & 0xFF), ((type >> 8) & 0xFF), ((type >> 16) & 0xFF), ((type >> 24) & 0xFF),
				version, length);
			if (type != 'HIST') continue;
			std::uint32_t jsonLength = 0;
			if (a_intfc->ReadRecordData(&jsonLength, sizeof(jsonLength)) != sizeof(jsonLength)) {
				LOG_error("!!! MyLoadCallback: Failed to read JSON string length.");
				continue;
			}
			std::string jsonString(jsonLength, '\0');
			if (a_intfc->ReadRecordData(&jsonString[0], jsonLength) != jsonLength) {
				LOG_error("!!! MyLoadCallback: Failed to read JSON string data.");
				continue;
			}
		
			if (DeserializeDialogueHistoryFromJSON(jsonString, a_intfc))
				LOG_info("MyLoadCallback: Successfully loaded dialogue history from F4SE co-save.");
			else
				LOG_error("!!! MyLoadCallback: Failed to deserialize dialogue history from JSON.");
		}
	} catch (const std::exception& e) {
		LOG_error("!!! MyLoadCallback: Exception during deserialization: {}", e.what());
	}

}

void MyRevertCallback(const F4SE::SerializationInterface*) {
	using MDT = Hooks::MantellaDialogueTracker;

	MDT::s_dialogueHistory.clear();
	MDT::s_preCacheByQuest.clear();
	Hooks::ShowSubtitle::s_lastPlayerTopicText.clear();
	MDT::lastCapturedGameTimeHours = 0.0f;
	LOG_info("MyRevertCallback: Cleared dialogue history.");
}
#pragma endregion


void OnGameDataReadyVanilla() {
	using MDT = Hooks::MantellaDialogueTracker;
	Hooks::SubtitleManagerShowSubtitleOrg = (void*)Hooks::SubtitleManagerShowSubtitleOffset.address();
	Hooks::SubtitleManagerShowSubtitleDetour.Create(
		Hooks::SubtitleManagerShowSubtitleOrg, Hooks::ShowSubtitle::SubtitleManagerShowSubtitleHook);

	MDT::Setup();
	//Hooks::ShowSubtitle::InitDialogueFilterCsv();

	LOG_info("F4SEPluginLoad: Installed ShowSubtitle hook.");

	auto serialization = F4SE::GetSerializationInterface();
	if (!serialization) {
		LOG_error("!!! OnF4SEMessage: Failed to get F4SE Serialization Interface.");
		return;
		}

	try {
		serialization->SetUniqueID(kSerializationID);
		serialization->SetSaveCallback(MySaveCallback);
		serialization->SetLoadCallback(MyLoadCallback);
		serialization->SetRevertCallback(MyRevertCallback);
		}
	catch (const std::exception& e) {
		LOG_error("!!! OnF4SEMessage: Exception during F4SE Serialization callbacks: {}", e.what());
		}

	LOG_info("OnF4SEMessage: Registered F4SE Serialization callbacks for dialogue history.");
	}

void OnPostLoadGameVanilla() {
	}

std::string GetVanillaDialogue(std::monostate) {
	LOG_info("GetVanillaDialogue called, returning {} characters", Hooks::dialogueEventsBuffer.size());
	std::string dialogueEvents = Hooks::dialogueEventsBuffer;
	Hooks::dialogueEventsBuffer.clear();
	return dialogueEvents;
	}

// Receive events from Mantella
void VanillaProcessMantellaEvent(
	std::string a_eventName,				// The name of the event being dispatched
	RE::Actor* target,						// Actor target of the action
	std::string stuff,						// Future use
	int handle) {							// JSON handle for action params
	(void)stuff; (void)handle;
	using MDT = Hooks::MantellaDialogueTracker;

	if (a_eventName == "MantellaConversation_Started") {
		//LOG_info("Vanilla:Conversation Started");
		if (!Hooks::IsEnabled()) return;
		MDT::OnConversationStarted();
		}
	else if (a_eventName == "MantellaConversation_NPC_Added") {
		//LOG_info("Vanilla:NPC added");
		MDT::OnNewParticipant(target);
		}
	else if (a_eventName == "MantellaConversation_NPC_Removed") {
		//LOG_debug("Vanilla:Actor left conversation");
		}
	else if (a_eventName == "MantellaConversation_Ended") {
		//LOG_info("Vanilla:Conversation Ended");
		}
	}

