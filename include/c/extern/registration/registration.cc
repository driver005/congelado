#include "c/extern/registration/registration.h"
#include "c/intern/tf_tstring.h"

#include <string>
#include <unordered_map>

namespace {

    std::string key(const TF_String* type, const TF_String* name)
    {
        const char* type_data = TF_StringGetDataPointer(type);
        size_t type_size = TF_StringGetSize(type);
        const char* name_data = TF_StringGetDataPointer(name);
        size_t name_size = TF_StringGetSize(name);
        std::string t = type_data ? std::string(type_data, type_size) : "";
        std::string n = name_data ? std::string(name_data, name_size) : "";
        return t + "/" + n;
    }

    std::unordered_map<std::string, void*>& storage()
    {
        static std::unordered_map<std::string, void*> map;
        return map;
    }

    void registration_register(void* plugin_context, const TF_String* type, const TF_String* name, void* value)
    {
        storage()[key(type, name)] = value;
    }

    void* registration_get(void* plugin_context, const TF_String* type, const TF_String* name)
    {
        auto it = storage().find(key(type, name));
        if (it == storage().end()) {
            return nullptr;
        }
        return it->second;
    }

    void registration_unregister(void* plugin_context, const TF_String* type, const TF_String* name)
    {
        storage().erase(key(type, name));
    }

} // namespace

extern "C" void TF_InitRegistration(TF_Registration** ops, void** plugin_context, TF_Status* status)
{
    static TF_Registration static_ops = {
        sizeof(TF_Registration),
        registration_register,
        registration_get,
        registration_unregister
    };
    *ops = &static_ops;
    if (plugin_context) *plugin_context = nullptr;
}
