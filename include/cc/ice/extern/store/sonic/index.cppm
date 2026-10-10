// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/index.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/index.h"

export module cc_ice_extern_store_sonic:index;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFStoreIndexOps : public ice::sonic::Runtime<::TFStoreIndexOps, ::TFStoreIndex>
{
public:
    TFStoreIndexOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFStoreIndexOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFStoreIndex* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFStoreIndexOps(const ::TFStoreIndexOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFStoreIndexOps(const ::TFStoreIndexOps* ops, ::TFStoreIndex* handle) noexcept :
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

    void create_index(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& field_config,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_index(
            get_handle(),
            name.get_handle(),
            field_config.get_handle(),
            out_status.get_handle()
        );
    }

    void drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->drop(get_handle(), name.get_handle(), out_status.get_handle());
    }

    void list(
        const ice::sonic::TF_VectorOps& out_names,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list(get_handle(), out_names.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
