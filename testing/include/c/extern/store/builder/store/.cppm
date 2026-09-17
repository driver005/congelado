// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/store.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/store.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_StoreOps
{
public:
    static TF_StoreOps* create(void* ctx) noexcept
    {
        return static_cast<TF_StoreOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StoreOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_StoreOps*>(handle->plugin_data);
    }

    virtual ~TF_StoreOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_connected(int* out_connected) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> backup(
        const ice::sonic::TF_StringOps& destination,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> restore(
        const ice::sonic::TF_StringOps& source,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;

    static TF_StoreOps* get_generic_vtable()
    {
        static TF_StoreOps vtable = {
            .struct_size = TF_STORE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_StoreOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_StoreOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .is_connected =
                [](TF_Store* store, int* out_connected) noexcept
            {
                auto* self = TF_StoreOps::create(store);
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
                auto* self = TF_StoreOps::create(store);
                auto res = self->backup(
                    ice::sonic::TF_StringOps::wrap(destination),
                    completion,
                    user_data
                );
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
                auto* self = TF_StoreOps::create(store);
                auto res =
                    self->restore(ice::sonic::TF_StringOps::wrap(source), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
