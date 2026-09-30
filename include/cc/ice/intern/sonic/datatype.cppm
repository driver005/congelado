// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/datatype.h"

export module cc_ice_intern_sonic:datatype;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_DataTypeOps : public ice::sonic::Runtime<TF_DataTypeOps, TF_DataTypeOps>
{
public:
    explicit TF_DataTypeOps(TF_DataTypeOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void datatype_size(TFDataTypeEnum dt, size_t* out_size) noexcept
    {
        m_ops->datatype_size(get_handle(), dt, out_size);
    }
};

} // namespace ice::sonic
