// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/admin.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/admin.h"

export module cc_ice_builder_store:admin;

import std;

export namespace ice::builder {

class TFStoreAdminOps
{
public:
    static TFStoreAdminOps* create(void* ctx) noexcept
    {
        return static_cast<TFStoreAdminOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreAdminOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFStoreAdminOps*>(handle->plugin_data);
    }

    virtual ~TFStoreAdminOps() = default;
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

    static TFStoreAdminOps* get_generic_vtable()
    {
        static TFStoreAdminOps vtable = {
            .struct_size = TF_TOREADMIN_STRUCT_SIZE,
            .is_connected =
                [](TFStoreAdmin* manager, int* out_connected) noexcept
            {
                auto* self = TFStoreAdminOps::create(manager);
                auto res = self->is_connected(out_connected);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .backup =
                [](TFStoreAdmin* manager,
                   const TF_String* destination,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreAdminOps::create(manager);
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
                [](TFStoreAdmin* manager,
                   const TF_String* source,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreAdminOps::create(manager);
                auto res =
                    self->restore(ice::sonic::TF_StringOps::wrap(source), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
