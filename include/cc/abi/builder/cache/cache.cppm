module;

#include "c/extern/cache/cache.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_cache;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Cache
{
public:
    static Cache* create(void* ctx) noexcept
    {
        return static_cast<Cache*>(ctx);
    }

    virtual ~Cache() = default;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(const ice::String& key, TF_Cache_CompletionFn completion, void* cb_user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set(const ice::String& key,
        const ice::String& value,
        TF_Cache_CompletionFn completion,
        void* cb_user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> remove(
        const ice::String& key,
        TF_Cache_CompletionFn completion,
        void* cb_user_data
    ) noexcept = 0;

    virtual ice::String get_name() const noexcept = 0;

    static TF_Cache* get_generic_vtable()
    {
        static TF_Cache vtable = {
            .struct_size = TF_CACHE_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Cache::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Cache::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .get =
                [](void* plugin_context,
                   const TF_TString* key,
                   TF_Cache_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status) noexcept
            {
                auto* self = Cache::create(plugin_context);
                auto res = self->get(ice::String::create(key), completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set =
                [](void* plugin_context,
                   const TF_TString* key,
                   const TF_TString* value,
                   TF_Cache_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status) noexcept
            {
                auto* self = Cache::create(plugin_context);
                auto res = self->set(
                    ice::String::create(key),
                    ice::String::create(value),
                    completion,
                    cb_user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove =
                [](void* plugin_context,
                   const TF_TString* key,
                   TF_Cache_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status) noexcept
            {
                auto* self = Cache::create(plugin_context);
                auto res = self->remove(ice::String::create(key), completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
        };
        return &vtable;
    }
};

} // namespace ice::builder
