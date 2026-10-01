// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/store.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/store.h"

export module cc_ice_extern_store_sonic:store;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_StoreOps : public ice::sonic::Runtime<::TF_StoreOps, ::TF_Store>
{
public:
    template<typename Registry>
    TF_StoreOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_StoreOps(
        Registry& registry,
        ::TF_Store* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_StoreOps(const ::TF_StoreOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_StoreOps(const ::TF_StoreOps* ops, ::TF_Store* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }
};

} // namespace ice::sonic
