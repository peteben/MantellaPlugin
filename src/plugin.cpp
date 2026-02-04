#include <stdio.h>

#define DLLEXPORT __declspec(dllexport)

extern void initDetours(void);
extern void initSettings();
extern void RegisterCrosshair();
extern bool RegisterInfoPatcherFunctions(std::string moduleName, RE::BSScript::IVirtualMachine* a_VM);
extern bool RegisterHTTPFunctions(std::string moduleName, RE::BSScript::IVirtualMachine* a_VM);
extern bool RegisterLauncherFunctions(std::string moduleName, RE::BSScript::IVirtualMachine* a_vm);

void init_log() {
    std::optional<std::filesystem::path> logpath = logger::log_directory();

    const char* plugin_name = "MantellaPlugin";
    *logpath /= fmt::format(FMT_STRING("{}.log"), plugin_name);
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logpath->string(), true);

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

    log->set_level(spdlog::level::trace);
    log->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%T.%e] [%=5t] [%L] %v"s);
    }

void MessageHandler(F4SE::MessagingInterface::Message* a_msg) {
    if (!a_msg) {
        return;
        }

    switch (a_msg->type) {

        case F4SE::MessagingInterface::kGameDataReady:
        {
        logger::info("GameDataReady");
        initSettings();
        }
        break;
        case F4SE::MessagingInterface::kPostLoadGame:
        case F4SE::MessagingInterface::kNewGame:
            if (!REL::Module::IsVR()) {
                RegisterCrosshair();
                }
            break;
        }
    }


bool Bind(RE::BSScript::IVirtualMachine* vm) {
    std::string className = "MantellaPlugin";

    RegisterInfoPatcherFunctions(className, vm);
    RegisterHTTPFunctions(className, vm);
    RegisterLauncherFunctions(className, vm);

     return true;
};

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info) {
    a_info->infoVersion = F4SE::PluginInfo::kVersion;
    a_info->name = "MantellaPlugin";
    a_info->version = 16;
    return true;
};

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se) {
    F4SE::Init(a_f4se);

    init_log();

    F4SE::GetPapyrusInterface()->Register(Bind);
    F4SE::GetMessagingInterface()->RegisterListener(MessageHandler);
	
    logger::info("MantellaPlugin loaded");

    return true;
};

// Only used by F4SE 0.7.1 and above

extern "C" DLLEXPORT  constinit auto F4SEPlugin_Version = []() noexcept {
    F4SE::PluginVersionData data{};

    data.PluginName("MantellaPlugin");
    data.PluginVersion(REL::Version(1,0,0));
    data.AuthorName("Peteben");
    data.UsesAddressLibrary(true);
    data.UsesSigScanning(false);
    data.IsLayoutDependent(true);
    data.HasNoStructUse(false);
    data.CompatibleVersions({ F4SE::RUNTIME_LATEST, F4SE::RUNTIME_LATEST_VR, F4SE::RUNTIME_1_10_163 });

    return data;
}();

