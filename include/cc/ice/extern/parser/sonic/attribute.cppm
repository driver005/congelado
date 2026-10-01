// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/attribute.h"

export module cc_ice_extern_parser_sonic:attribute;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserAttributeOps : public ice::sonic::Runtime<::TFParserAttributeOps, ::TFParserAttribute>
{
public:
    template<typename Registry>
    TFParserAttributeOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFParserAttributeOps(
        Registry& registry,
        ::TFParserAttribute* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFParserAttributeOps(const ::TFParserAttributeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserAttributeOps(const ::TFParserAttributeOps* ops, ::TFParserAttribute* handle) noexcept :
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

    void get_name(
        const ice::sonic::String& out_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle(), out_status.get_handle());
    }

    void get_value(TF_Tensor** out_value, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_value(get_handle(), out_value, out_status.get_handle());
    }
};

} // namespace ice::sonic
