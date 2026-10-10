// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/typeinfo.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_parser_sonic:typeinfo;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserTypeInfoOps : public ice::sonic::Runtime<::TFParserTypeInfoOps, ::TFParserTypeInfo>
{
public:
    TFParserTypeInfoOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFParserTypeInfoOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFParserTypeInfo* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFParserTypeInfoOps(const ::TFParserTypeInfoOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserTypeInfoOps(const ::TFParserTypeInfoOps* ops, ::TFParserTypeInfo* handle) noexcept :
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

    void get_dtype(int* out_dtype, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_dtype(get_handle(), out_dtype, out_status.get_handle());
    }

    void get_shape(
        int64_t** out_dims,
        int* out_num_dims,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_shape(get_handle(), out_dims, out_num_dims, out_status.get_handle());
    }
};

} // namespace ice::sonic
