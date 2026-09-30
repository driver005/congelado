// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/dimension_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"

export module cc_ice_extern_ops_sonic:dimension_handle;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_DimensionHandleOps :
    public ice::sonic::Runtime<TF_DimensionHandleOps, TF_DimensionHandleOps>
{
public:
    explicit TF_DimensionHandleOps(TF_DimensionHandleOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "ops";

    void value_known(int* out_known) noexcept
    {
        m_ops->value_known(get_handle(), out_known);
    }

    void value(int64_t* out_value) noexcept
    {
        m_ops->value(get_handle(), out_value);
    }
};

} // namespace ice::sonic
