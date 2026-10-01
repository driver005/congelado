// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parameter.h"

export module cc_ice_extern_parser_sonic:parameter;

import std;
import :typeinfo;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserParameterOps : public ice::sonic::Runtime<::TFParserParameterOps, ::TFParserParameter>
{
public:
    template<typename Registry>
    TFParserParameterOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFParserParameterOps(
        Registry& registry,
        ::TFParserParameter* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFParserParameterOps(const ::TFParserParameterOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserParameterOps(const ::TFParserParameterOps* ops, ::TFParserParameter* handle) noexcept :
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

    void get_typeinfo(
        const ice::sonic::TFParserTypeInfoOps& out_typeinfo,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_typeinfo(get_handle(), out_typeinfo.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
