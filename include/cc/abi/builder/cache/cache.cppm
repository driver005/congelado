module;

#include "c/extern/cache/cache.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_cache;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Pure C++ callback shape for async cache operations — decoupled from the C ABI's
// TF_Cache_CompletionFn (which signals across a TF_TString*-based boundary); the C-ABI adapter
// (cc/abi/sonic/cache) is what bridges between the two when crossing into a cross-plugin
// implementation. result is empty if the key wasn't found.
using CompletionFn = void (*)(const ice::String& result, void* user_data);

inline CompletionFn completion_from_c(TF_Cache_CompletionFn fn) noexcept
{
    static_assert(sizeof(CompletionFn) == sizeof(TF_Cache_CompletionFn));
    return std::bit_cast<CompletionFn>(fn);
}

inline TF_Cache_CompletionFn completion_to_c(CompletionFn fn) noexcept
{
    static_assert(sizeof(TF_Cache_CompletionFn) == sizeof(CompletionFn));
    return std::bit_cast<TF_Cache_CompletionFn>(fn);
}

// Abstract base class for a cache backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Builder's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::RegistrationRuntime under type="cache"; this module
// never imports any specific backend implementation.
class Cache
{
public:
    virtual ~Cache() = default;

    virtual std::expected<void, ice::Status>
    get(const ice::String& key, CompletionFn completion, void* cb_user_data) = 0;

    virtual std::expected<void, ice::Status>
    set(const ice::String& key,
        const ice::String& value,
        CompletionFn completion,
        void* cb_user_data) = 0;

    virtual std::expected<void, ice::Status>
    remove(const ice::String& key, CompletionFn completion, void* cb_user_data) = 0;

    virtual ice::String get_name() const = 0;

    TF_Cache* get_generic_vtable()
    {
        static TF_Cache vtable = {
            .struct_size = sizeof(TF_Cache),
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Cache>(ctx);
                },
            .get_name =
                [](void* ctx, TF_String* out) {
                    auto* self = ctx_as<Cache>(ctx);
                    auto name = self->get_name();
                    name.to_c(out);
                },
            .get =
                [](void* ctx, const TF_TString* key, TF_Cache_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Cache>(ctx);
                    auto res = self->get(
                        ice::String::create(key), completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .set =
                [](void* ctx, const TF_TString* key, const TF_TString* value,
                   TF_Cache_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Cache>(ctx);
                    auto res = self->set(
                        ice::String::create(key), ice::String::create(value),
                        completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .remove =
                [](void* ctx, const TF_TString* key, TF_Cache_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Cache>(ctx);
                    auto res = self->remove(
                        ice::String::create(key), completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                }
        };
        return &vtable;
    }
};

} // namespace ice::builder
