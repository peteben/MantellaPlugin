#include <stdio.h>
#include <cpr\cpr.h>
#include <Mantella_TypedDictionary.h>
#include <nlohmann\json.hpp>
#include "RE\Bethesda\SendPapyrusEvent.h"

using json = nlohmann::json;
using namespace Mantella_TypedDictionary;

void toLowerCase(std::string* input) {
    std::transform(input->begin(), input->end(), input->begin(), [](unsigned char c) { return std::tolower(c); });
    };

bool test_utf8(std::string input) {
    try {
        json test = { "test", input };
        test.dump();
        return true;
        }
    catch (...) {
        return false;
        }
    };

json getJsonFromHandle(int typedDictionaryHandle) {
    std::shared_ptr<TypedDictionary> dict = dicNestedDictionariesValues[typedDictionaryHandle];
    json jsonToUse;
    if (dict) {
        for (auto& [key, value] : dict->_dicElements) {
            std::string valueType = value->getTypeName();
            if (valueType == "string")
                jsonToUse[key] = dict->getString(key);
            else if (valueType == "int")
                jsonToUse[key] = dict->getInt(key);
            else if (valueType == "float")
                jsonToUse[key] = dict->getFloat(key);
            else if (valueType == "bool")
                jsonToUse[key] = dict->getBool(key);
            else if (valueType == "stringArray")
                jsonToUse[key] = dict->getStringArray(key);
            else if (valueType == "intArray")
                jsonToUse[key] = dict->getIntArray(key);
            else if (valueType == "floatArray")
                jsonToUse[key] = dict->getFloatArray(key);
            else if (valueType == "boolArray")
                jsonToUse[key] = dict->getBoolArray(key);
            else if (valueType == "NestedDictionary") {
                int handle = dict->getNestedDictionary(key);
                jsonToUse[key] = getJsonFromHandle(handle);
                }
            else if (valueType == "NestedDictionaryArray") {
                std::vector<int> handles = dict->getArrayOfNestedDictionaries(key);
                auto jsonObjects = json::array();
                size_t sizeOfHandles = handles.size();
                for (auto i = 0; i < sizeOfHandles; ++i) {
                    jsonObjects.push_back(getJsonFromHandle(handles[i]));
                    }
                jsonToUse[key] = jsonObjects;
                }
            }
        }
    return jsonToUse;
    };

int generateDictionaryFromJson(json jsonToUse) {
    int handle = createDictionary();
    for (auto& el : jsonToUse.items()) {
        std::string key = el.key();
        toLowerCase(&key);
        if (el.value().is_string())
            setString(handle, key, el.value());
        else if (el.value().is_number_integer())
            setInt(handle, key, el.value());
        else if (el.value().is_number_float())
            setFloat(handle, key, el.value());
        else if (el.value().is_boolean())
            setBool(handle, key, el.value());
        else if (el.value().is_object()) {
            json nested = el.value();
            int subHandle = generateDictionaryFromJson(nested);
            setNestedDictionary(handle, key, subHandle);
            }
        else if (el.value().is_array()) {
            if (std::all_of(el.value().begin(), el.value().end(), [](const json& elSub) { return elSub.is_string(); }))
                setStringArray(handle, key, el.value());
            else if (std::all_of(el.value().begin(), el.value().end(), [](const json& elSub) { return elSub.is_number_integer(); }))
                setIntArray(handle, key, el.value());
            else if (std::all_of(el.value().begin(), el.value().end(), [](const json& elSub) { return elSub.is_number_float(); }))
                setFloatArray(handle, key, el.value());
            else if (std::all_of(el.value().begin(), el.value().end(), [](const json& elSub) { return elSub.is_boolean(); }))
                setBoolArray(handle, key, el.value());
            else if (std::all_of(el.value().begin(), el.value().end(), [](const json& elSub) { return elSub.is_object(); })) {
                std::vector<int> handles;
                for (auto& elSub : el.value().items()) {
                    json nested = elSub.value();
                    int subHandle = generateDictionaryFromJson(nested);
                    handles.push_back(subHandle);
                    }
                setArrayOfNestedDictionaries(handle, key, handles);
                }
            }

        }
    return handle;
    };


// Notify game that data is ready to be consumed

int sendHttpRequestResultToGameEvent(std::string completeReply, std::string eventName) {
    try {
        json reply = json::parse(completeReply);
        int handle = generateDictionaryFromJson(reply);

        //logger::info("{}: [{:d}] {}", eventName, handle, getString(handle, "mantella_reply_type", "none"));
        Papyrus::detail::SendPapyrusExternalEvent(eventName, handle);

        return 0;
        }
    catch (...) {
        return 1;
        }
    }

void postCallbackMethod(cpr::Response response) {
    if (response.status_code == 200) {
        std::string HttpReplyReceived = "HttpReplyReceived";
        sendHttpRequestResultToGameEvent(response.text, HttpReplyReceived);
        }
    else {
        json jsonToUse;
        logger::info("Error {}: {} ", response.status_code, response.error.message);
        jsonToUse["F4SE_HTTP_error"] = response.error.message;
        std::string HttpErrorReceived = "HttpErrorReceived";
        sendHttpRequestResultToGameEvent(jsonToUse.dump(), HttpErrorReceived);
        }
    }

void sendLocalhostHttpRequest(std::monostate,
    int typedDictionaryHandle, int port,
    std::string route, int timeout) {
    try {
        toLowerCase(&route);
        json newJson = getJsonFromHandle(typedDictionaryHandle);
        std::string textToSend = newJson.dump();
        std::string req_type = getString(typedDictionaryHandle, "mantella_request_type", "none");

        //logger::info("SEND: {} --> {}", req_type, port);

        //if (req_type == "mantella_start_conversation") {            // Experimental feature, turn radio off
        //    auto* vm = RE::GameVM::GetSingleton()->GetVM().get();   // During conversation

        //    Papyrus::detail::DispatchStaticCall(
        //        vm,
        //        "Game",
        //        "TurnPlayerRadioOn",
        //        nullptr,
        //        false);
        //    RE::SendHUDMessage::ShowHUDMessage("Radio OFF", "", false, false);
        //    }
        //else if (req_type == "mantella_end_conversation") {
        //    auto* vm = RE::GameVM::GetSingleton()->GetVM().get(); 

        //    Papyrus::detail::DispatchStaticCall(
        //        vm,
        //        "Game",
        //        "TurnPlayerRadioOn",
        //        nullptr,
        //        true);
        //    RE::SendHUDMessage::ShowHUDMessage("Radio ON", "", false, false);
        //    }

        std::string url = "http://localhost:" + std::to_string(port) + "/" + route;
        //logger::info("Http send: {} : {}", url, textToSend);
            cpr::PostCallback(postCallbackMethod,
            cpr::Url{ url },
            cpr::ConnectTimeout{ timeout },
            cpr::Authentication{ "user", "pass", cpr::AuthMode::BASIC },
            cpr::Header{ {"Content-Type", "application/json"} },
            cpr::Header{ {"accept", "application/json"} },
            cpr::Body{ textToSend });
        }
    catch (...) {

        }
    };

void clearAllDictionaries(std::monostate) {
    clearAll();
    }

int createDictionaryRelay(std::monostate) {
    return createDictionary();
    };

// Returns the value associated with the @key. If not, returns @default value
std::string getStringRelay(std::monostate, int object, std::string key, std::string defaultValue) {
    toLowerCase(&key);
    std::string val = getString(object, key, defaultValue);
    //logger::info("  GetString {} {}", key, val);
    return val;
    };
int getIntRelay(std::monostate, int object, std::string key, int defaultValue) {
    toLowerCase(&key);
    return getInt(object, key, defaultValue);
    };
float getFloatRelay(std::monostate, int object, std::string key, float defaultValue) {
    toLowerCase(&key);
    return getFloat(object, key, defaultValue);
    };
bool getBoolRelay(std::monostate, int object, std::string key, bool defaultValue) {
    toLowerCase(&key);
    return getBool(object, key, defaultValue);
    };
int getNestedDictionaryRelay(std::monostate, int object,
    std::string key, int defaultValue) {
    toLowerCase(&key);
    return getNestedDictionary(object, key, defaultValue);
    };
std::vector<std::string> getStringArrayRelay(std::monostate, int object, std::string key) {
    toLowerCase(&key);
    return getStringArray(object, key);
    };
std::vector<int> getIntArrayRelay(std::monostate,
    int object, std::string key) {
    toLowerCase(&key);
    return getIntArray(object, key);
    };
std::vector<float> getFloatArrayRelay(std::monostate,
    int object, std::string key) {
    toLowerCase(&key);
    return getFloatArray(object, key);
    };
std::vector<bool> getBoolArrayRelay(std::monostate,
    int object, std::string key) {
    toLowerCase(&key);
    return getBoolArray(object, key);
    };
std::vector<int> getNestedDictionariesArrayRelay(std::monostate, int object, std::string key) {
    toLowerCase(&key);
    return getArrayOfNestedDictionaries(object, key);
    };

// Inserts @key: @value pair. Replaces existing pair with the same @key

bool setStringRelay(std::monostate, int object,
    std::string key, std::string value) {
    toLowerCase(&key);
    //std::string before = getJsonFromHandle(object).dump();
    //logger::info("SetString into: {:d} {} {}", object, key, value);
    //logger::info("Before: {}", before);
    if (!test_utf8(value)) return false;
    setString(object, key, value);
    //std::string after = getJsonFromHandle(object).dump();
    //logger::info("After: {}", after);
    return true;
    };
void setIntRelay(std::monostate, int object, std::string key, int value) {
    toLowerCase(&key);
    setInt(object, key, value);
    };
void setFloatRelay(std::monostate, int object, std::string key, float value) {
    toLowerCase(&key);
    setFloat(object, key, value);
    };
void setBoolRelay(std::monostate, int object, std::string key, bool value) {
    toLowerCase(&key);
    setBool(object, key, value);
    };
void setNestedDictionaryRelay(std::monostate, int object, std::string key, int value) {
    toLowerCase(&key);
    //std::string before = getJsonFromHandle(object).dump();
    //logger::info("SetNestedDict into: {:d} {} {:d}", object, key, value);
    //logger::info("Before: {}", before);
    setNestedDictionary(object, key, value);
    //std::string after = getJsonFromHandle(object).dump();
    //logger::info("After: {}", after);

    };
bool setStringArrayRelay(std::monostate, int object, std::string key, const std::vector<std::string> value) {
    toLowerCase(&key);
    std::vector<std::string> vector;
    bool result = true;
    //std::string before = getJsonFromHandle(object).dump();
    //logger::info("SetStringArray into: {:d} {}", object, key);
    //logger::info("Before: {}", before);
    try {
        for (int i = 0; i < value.size(); ++i) {
            if (test_utf8(value[i])) {
                vector.push_back(value[i]);
                }
            else {
                result = false;
                }
            }
        }
    catch (...) {
        }
    setStringArray(object, key, vector);
    //std::string after = getJsonFromHandle(object).dump();
    //logger::info("After: {}", after);
    return result;
    };
void setIntArrayRelay(std::monostate, int object,
    std::string key, const std::vector<int> value) {
    toLowerCase(&key);
    std::vector<int> vector;
    try {
        for (int i = 0; i < value.size(); ++i) vector.push_back(value[i]);
        }
    catch (...) {
        }
    setIntArray(object, key, vector);
    };
void setFloatArrayRelay(std::monostate, int object,
    std::string key, const std::vector<float> value) {
    toLowerCase(&key);
    std::vector<float> vector;
    try {
        for (int i = 0; i < value.size(); ++i) vector.push_back(value[i]);
        }
    catch (...) {
        }
    setFloatArray(object, key, vector);
    };
void setBoolArrayRelay(std::monostate, int object, std::string key, std::vector<bool> value) {
    toLowerCase(&key);
    std::vector<bool> vector;
    try {
        for (int i = 0; i < value.size(); ++i) vector.push_back(value[i]);
        }
    catch (...) {
        }
    setBoolArray(object, key, vector);
    };
void setNestedDictionariesArrayRelay(std::monostate, int object, std::string key, const std::vector<int> value) {
    toLowerCase(&key);
    std::vector<int> vector;
    try {
        for (int i = 0; i < value.size(); ++i) vector.push_back(value[i]);
        }
    catch (...) {
        }
    setArrayOfNestedDictionaries(object, key, vector);
    };

//  Returns true, if the container has @key: value pair
bool hasKeyRelay(std::monostate, int object, std::string key) { return hasKey(object, key); };



bool RegisterHTTPFunctions(std::string moduleName, RE::BSScript::IVirtualMachine* vm) {
    vm->BindNativeMethod(moduleName, "sendLocalhostHttpRequest", sendLocalhostHttpRequest);

    vm->BindNativeMethod(moduleName, "createDictionary", createDictionaryRelay);
    vm->BindNativeMethod(moduleName, "clearAllDictionaries", clearAllDictionaries);

    vm->BindNativeMethod(moduleName, "getString", getStringRelay);
    vm->BindNativeMethod(moduleName, "getInt", getIntRelay);
    vm->BindNativeMethod(moduleName, "getFloat", getFloatRelay);
    vm->BindNativeMethod(moduleName, "getBool", getBoolRelay);
    vm->BindNativeMethod(moduleName, "getNestedDictionary", getNestedDictionaryRelay);
    vm->BindNativeMethod(moduleName, "getStringArray", getStringArrayRelay);
    vm->BindNativeMethod(moduleName, "getIntArray", getIntArrayRelay);
    vm->BindNativeMethod(moduleName, "getFloatArray", getFloatArrayRelay);
    vm->BindNativeMethod(moduleName, "getBoolArray", getBoolArrayRelay);
    vm->BindNativeMethod(moduleName, "getNestedDictionariesArray", getNestedDictionariesArrayRelay);

    vm->BindNativeMethod(moduleName, "setString", setStringRelay);
    vm->BindNativeMethod(moduleName, "setInt", setIntRelay);
    vm->BindNativeMethod(moduleName, "setFloat", setFloatRelay);
    vm->BindNativeMethod(moduleName, "setBool", setBoolRelay);
    vm->BindNativeMethod(moduleName, "setNestedDictionary", setNestedDictionaryRelay);
    vm->BindNativeMethod(moduleName, "setStringArray", setStringArrayRelay);
    vm->BindNativeMethod(moduleName, "setIntArray", setIntArrayRelay);
    vm->BindNativeMethod(moduleName, "setFloatArray", setFloatArrayRelay);
    vm->BindNativeMethod(moduleName, "setBoolArray", setBoolArrayRelay);
    vm->BindNativeMethod(moduleName, "setNestedDictionariesArray", setNestedDictionariesArrayRelay);

    //vm->BindNativeMethod(moduleName, "GetHandle", GetHandle);
    vm->BindNativeMethod(moduleName, "hasKey", hasKeyRelay);
    //a_VM->BindNativeMethod(moduleName, "GetActorName", GetActorName, true);
	return true;
	}
