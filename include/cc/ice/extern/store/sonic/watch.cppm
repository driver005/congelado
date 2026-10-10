// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/watch.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/watch.h"

export module cc_ice_extern_store_sonic:watch;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFStoreWatchOps : public ice::sonic::Runtime<::TFStoreWatchOps, ::TFStoreWatch>
{
public:
    TFStoreWatchOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFStoreWatchOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFStoreWatch* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFStoreWatchOps(const ::TFStoreWatchOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFStoreWatchOps(const ::TFStoreWatchOps* ops, ::TFStoreWatch* handle) noexcept :
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

    void cancel() const noexcept
    {
        m_ops->cancel(get_handle());
    }
};

} // namespace ice::sonic
