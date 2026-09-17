// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/store/store.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/store/store.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Store
{
public:
    static Store* create(void* ctx) noexcept
    {
        return static_cast<Store*>(ctx);
    }

    template<typename HandleT>
    static Store* create(HandleT* handle) noexcept
    {
        return static_cast<Store*>(handle->plugin_data);
    }

    virtual ~Store() = default;
    [[nodiscard]] std::expected<void, ice::Status> is_connected(int* out_connected) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> backup(
        const ice::sonic::String& destination,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    restore(const ice::sonic::String& source, TFStoreAckFn completion, void* user_data) noexcept =
        0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_StoreOps* get_generic_vtable()
    {
        static TF_StoreOps vtable = {
            .struct_size = TF_STORE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Store::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .is_connected =
                [](TF_Store* store, int* out_connected) noexcept
            {
                auto* self = Store::create(store);
                auto res = self->is_connected(out_connected);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .backup =
                [](TF_Store* store,
                   const TF_String* destination,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = Store::create(store);
                auto res =
                    self->backup(ice::sonic::String::wrap(destination), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .restore =
                [](TF_Store* store,
                   const TF_String* source,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = Store::create(store);
                auto res = self->restore(ice::sonic::String::wrap(source), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
