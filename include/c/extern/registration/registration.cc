#include "c/extern/registration/registration.h"

#include "c/intern/tf_tstring.h"

#include <mutex>
#include <string>
#include <unordered_map>

// Process-wide named-value registry backing c/extern/registration/registration.h.
//
// Fixes vs the previous implementation:
//  * the exported symbol is `init_registration`, exactly as registration.h declares it
//    (the old .cc defined `TF_InitRegistration`, which nothing declared or linked);
//  * the vtable entry points match the header's signatures exactly (register_op, get and
//    unregister all take const TF_TString* type/name), and the vtable carries the standard
//    destroy/get_name prologue slots;
//  * the string accessors are string_get_data_pointer/string_get_size from tf_tstring.h
//    (TF_StringGetDataPointer/TF_StringGetSize do not exist);
//  * every entry point is noexcept: no C++ exception may escape a C ABI function, so the
//    allocating std::string/unordered_map paths are wrapped in a leaf catch that degrades
//    to an empty key / null result instead of throwing;
//  * access is serialized with a mutex (the previous unordered_map was a data race when
//    a plugin thread and the host thread registered/looked up concurrently).

namespace {

std::mutex& registry_mutex()
{
    static std::mutex m;
    return m;
}

std::unordered_map<std::string, void*>& storage()
{
    static std::unordered_map<std::string, void*> map;
    return map;
}

// Key builders — allocation can throw (bad_alloc); noexcept contract requires the leaf
// catch, degrading to an empty key (which simply won't match anything) on OOM.

std::string key_from_tstring(const TF_TString* type, const TF_TString* name) noexcept
{
    try {
        const char* type_data = string_get_data_pointer(type);
        size_t type_size = string_get_size(type);
        std::string t{type_data ? type_data : "", type_size};
        const char* name_data = string_get_data_pointer(name);
        size_t name_size = string_get_size(name);
        std::string n{name_data ? name_data : "", name_size};
        return t + "/" + n;
    } catch (...) {
        return {};
    }
}

void registration_register(void* plugin_context, const TF_TString* type, const TF_TString* name, void* value) noexcept
{
    try {
        std::lock_guard<std::mutex> lock{registry_mutex()};
        storage()[key_from_tstring(type, name)] = value;
    } catch (...) {
        // no exception may escape a C ABI function
    }
}

void* registration_get(void* plugin_context, const TF_TString* type, const TF_TString* name) noexcept
{
    try {
        std::lock_guard<std::mutex> lock{registry_mutex()};
        auto it = storage().find(key_from_tstring(type, name));
        return it == storage().end() ? nullptr : it->second;
    } catch (...) {
        return nullptr;
    }
}

void registration_unregister(void* plugin_context, const TF_TString* type, const TF_TString* name) noexcept
{
    try {
        std::lock_guard<std::mutex> lock{registry_mutex()};
        storage().erase(key_from_tstring(type, name));
    } catch (...) {
        // no exception may escape a C ABI function
    }
}

} // namespace

extern "C" void init_registration(TF_Registration** ops, void** plugin_context, TF_Status* status)
{
    static TF_Registration static_ops = {
        .struct_size = TF_REGISTRATION_STRUCT_SIZE,
        .destroy = [](void* plugin_context) noexcept {},
        .get_name = [](void* plugin_context, TF_String* out) noexcept {
            if (out) {
                string_init(out);
                string_copy(out, "registration", 12);
            }
        },
        .register_op = registration_register,
        .get = registration_get,
        .unregister = registration_unregister,
    };
    *ops = &static_ops;
    if (plugin_context) {
        *plugin_context = nullptr;
    }
}
