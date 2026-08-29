module;

#include "c/extern/database/database.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_database;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Database
{
public:
    // Recover the Database instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Database* create(void* ctx) noexcept
    {
        return static_cast<Database*>(ctx);
    }

    virtual ~Database() = default;

    virtual [[nodiscard]] std::expected<bool, ice::Status> is_connected() = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    query(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    insert(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    update(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    remove(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) = 0;

    virtual ice::String get_name() const = 0;

    static TF_Database* get_generic_vtable()
    {
        static TF_Database vtable = {
            .struct_size = sizeof(TF_Database),
            .destroy =
                [](void* plugin_context)
            {
                delete Database::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Database::create(plugin_context);
                auto name = self->get_name();

                name.to_c(out);
            },
            .is_connected = [](void* plugin_context, TF_Status* status) -> TF_Bool
            {
                auto* self = Database::create(plugin_context);
                auto res = self->is_connected();
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .query =
                [](void* plugin_context,
                   const TF_TString* payload,
                   TF_Database_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Database::create(plugin_context);
                ice::String payload_str(payload);
                auto res = self->query(payload_str, completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .insert =
                [](void* plugin_context,
                   const TF_TString* payload,
                   TF_Database_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Database::create(plugin_context);
                ice::String payload_str(payload);
                auto res = self->insert(payload_str, completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .update =
                [](void* plugin_context,
                   const TF_TString* payload,
                   TF_Database_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Database::create(plugin_context);
                ice::String payload_str(payload);
                auto res = self->update(payload_str, completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove =
                [](void* plugin_context,
                   const TF_TString* payload,
                   TF_Database_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Database::create(plugin_context);
                ice::String payload_str(payload);
                auto res = self->remove(payload_str, completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
