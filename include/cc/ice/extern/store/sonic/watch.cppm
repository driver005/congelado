// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/watch.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/watch.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_store_sonic:watch;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class StoreWatch : public ice::sonic::Runtime<::TFStoreWatchOps, ::TFStoreWatch>
{
public:
    StoreWatch(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    StoreWatch(
        const ice::sonic::Registration& registry,
        ::TFStoreWatch* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit StoreWatch(const ::TFStoreWatchOps* ops) noexcept :
        Runtime(ops)
    {
    }

    StoreWatch(const ::TFStoreWatchOps* ops, ::TFStoreWatch* handle) noexcept :
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
