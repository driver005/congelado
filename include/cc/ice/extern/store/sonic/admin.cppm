// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/admin.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/admin.h"

export module cc_ice_extern_store_sonic:admin;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFStoreAdminOps : public ice::sonic::Runtime<::TFStoreAdminOps, ::TFStoreAdmin>
{
public:
    TFStoreAdminOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFStoreAdminOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFStoreAdmin* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFStoreAdminOps(const ::TFStoreAdminOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFStoreAdminOps(const ::TFStoreAdminOps* ops, ::TFStoreAdmin* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void is_connected(int* out_connected) const noexcept
    {
        m_ops->is_connected(get_handle(), out_connected);
    }

    void backup(
        const ice::sonic::String& destination,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->backup(
            get_handle(),
            destination.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void restore(
        const ice::sonic::String& source,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->restore(
            get_handle(),
            source.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
