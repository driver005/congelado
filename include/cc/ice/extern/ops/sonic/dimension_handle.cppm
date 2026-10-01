// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/dimension_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"

export module cc_ice_extern_ops_sonic:dimension_handle;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_DimensionHandleOps :
    public ice::sonic::Runtime<::TF_DimensionHandleOps, ::TF_DimensionHandle>
{
public:
    template<typename Registry>
    TF_DimensionHandleOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_DimensionHandleOps(
        Registry& registry,
        ::TF_DimensionHandle* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_DimensionHandleOps(const ::TF_DimensionHandleOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_DimensionHandleOps(const ::TF_DimensionHandleOps* ops, ::TF_DimensionHandle* handle) noexcept
        :
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

    void value_known(int* out_known) const noexcept
    {
        m_ops->value_known(get_handle(), out_known);
    }

    void value(int64_t* out_value) const noexcept
    {
        m_ops->value(get_handle(), out_value);
    }
};

} // namespace ice::sonic
