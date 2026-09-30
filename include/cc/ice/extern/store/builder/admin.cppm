// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/admin.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/admin.h"

export module cc_ice_extern_store_builder:admin;

import std;

export namespace ice::builder {

class TFStoreAdminOps
{
public:
    TFStoreAdminOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFStoreAdminOps(const TFStoreAdminOps&) = delete;
    TFStoreAdminOps& operator=(const TFStoreAdminOps&) = delete;

    static TFStoreAdminOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFStoreAdminOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreAdminOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFStoreAdminOps*>(handle->plugin_data);
    }

    virtual ~TFStoreAdminOps() = default;
    virtual void is_connected(int* out_connected) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> backup(
        const ice::sonic::String& destination,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> restore(
        const ice::sonic::String& source,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFStoreAdminOps{
            .struct_size = TF_TOREADMIN_STRUCT_SIZE,
            .is_connected =
                [](TFStoreAdmin* manager, int* out_connected) noexcept
            {
                TFStoreAdminOps::from_handle(manager).is_connected(out_connected);
            },
            .backup =
                [](TFStoreAdmin* manager,
                   const TF_String* destination,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TFStoreAdminOps::from_handle(manager).backup(
                    ice::sonic::String::wrap(destination),
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
                auto res = TFStoreAdminOps::from_handle(manager)
                               .restore(ice::sonic::String::wrap(source), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFStoreAdminOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFStoreAdmin& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFStoreAdminOps m_vtable;
    TFStoreAdmin m_handle;
};

} // namespace ice::builder
