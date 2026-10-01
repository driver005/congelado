// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/admin.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/admin.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_store_builder:admin;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFStoreAdminOps
{
public:
    explicit TFStoreAdminOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void is_connected(int* out_connected) noexcept = 0;
    virtual void backup(
        const ice::sonic::String& destination,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void restore(
        const ice::sonic::String& source,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFStoreAdmin*)) noexcept
    {
        m_vtable = ::TFStoreAdminOps{
            .struct_size = TF_OFFSET_OF_END(::TFStoreAdminOps, restore),

            .create = create,
            .destroy =
                [](TFStoreAdmin* handle) noexcept
            {
                auto& self = TFStoreAdminOps::from_handle(handle);
                self.destroy();
            },
            .is_connected =
                [](TFStoreAdmin* manager, int* out_connected) noexcept
            {
                auto& self = TFStoreAdminOps::from_handle(manager);
                self.is_connected(out_connected);
            },
            .backup =
                [](TFStoreAdmin* manager,
                   const TF_String* destination,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreAdminOps::from_handle(manager);
                self.backup(
                    self.wrap(std::type_identity<ice::sonic::String>{}, destination),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .restore =
                [](TFStoreAdmin* manager,
                   const TF_String* source,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreAdminOps::from_handle(manager);
                self.restore(
                    self.wrap(std::type_identity<ice::sonic::String>{}, source),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFStoreAdminOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreAdmin& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFStoreAdminOps*>(&m_vtable));
    }

private:
    ::TFStoreAdminOps m_vtable;
    ::TFStoreAdmin m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
