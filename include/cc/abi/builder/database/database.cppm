module;

#include "c/extern/database/database.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_database;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;


export namespace ice::builder {

using CompletionFn = void (*)(const ice::String& result, void* user_data);

inline CompletionFn completion_from_c(TF_Database_CompletionFn fn) noexcept {
    static_assert(sizeof(CompletionFn) == sizeof(TF_Database_CompletionFn));
    return std::bit_cast<CompletionFn>(fn);
}
inline TF_Database_CompletionFn completion_to_c(CompletionFn fn) noexcept {
    static_assert(sizeof(TF_Database_CompletionFn) == sizeof(CompletionFn));
    return std::bit_cast<TF_Database_CompletionFn>(fn);
}

class Database
{
public:
    virtual ~Database() = default;

    virtual std::expected<bool, ice::Status> is_connected() = 0;

    virtual std::expected<void, ice::Status> query(
        const ice::String& payload, CompletionFn completion, void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> insert(
        const ice::String& payload, CompletionFn completion, void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> update(
        const ice::String& payload, CompletionFn completion, void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> remove(
        const ice::String& payload, CompletionFn completion, void* cb_user_data
    ) = 0;

    virtual ice::String get_name() const = 0;

    TF_Database* get_generic_vtable()
    {
        static TF_Database vtable = {
            .struct_size = sizeof(TF_Database),
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Database>(ctx);
                },
            .get_name =
                [](void* ctx, TF_String* out) {
                    auto* self = ctx_as<Database>(ctx);
                    auto name = self->get_name();

                    name.to_c(out);
                },
            .is_connected = [](void* ctx, TF_Status* status) -> TF_Bool {
                auto* self = ctx_as<Database>(ctx);
                auto res = self->is_connected();
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .query =
                [](void* ctx, const TF_TString* payload, TF_Database_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Database>(ctx);
                    ice::String payload_str(payload);
                    auto res = self->query(
                        payload_str, completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .insert =
                [](void* ctx, const TF_TString* payload, TF_Database_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Database>(ctx);
                    ice::String payload_str(payload);
                    auto res = self->insert(
                        payload_str, completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .update =
                [](void* ctx, const TF_TString* payload, TF_Database_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Database>(ctx);
                    ice::String payload_str(payload);
                    auto res = self->update(
                        payload_str, completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .remove =
                [](void* ctx, const TF_TString* payload, TF_Database_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Database>(ctx);
                    ice::String payload_str(payload);
                    auto res = self->remove(
                        payload_str, completion_from_c(completion), cb_user_data
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
