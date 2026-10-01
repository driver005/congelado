// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/shape_handle.h"

export module cc_ice_extern_ops_sonic:shape_handle;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ShapeHandleOps : public ice::sonic::Runtime<::TF_ShapeHandleOps, ::TF_ShapeHandle>
{
public:
    template<typename Registry>
    TF_ShapeHandleOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ShapeHandleOps(
        Registry& registry,
        ::TF_ShapeHandle* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ShapeHandleOps(const ::TF_ShapeHandleOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ShapeHandleOps(const ::TF_ShapeHandleOps* ops, ::TF_ShapeHandle* handle) noexcept :
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
};

} // namespace ice::sonic
