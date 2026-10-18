// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parameter.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_parser_sonic:parameter;

import std;
import :typeinfo;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class ParserParameter : public ice::sonic::Runtime<::TFParserParameterOps, ::TFParserParameter>
{
public:
    ParserParameter(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    ParserParameter(
        const ice::sonic::Registration& registry,
        const ::TFParserParameter& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit ParserParameter(const ::TFParserParameterOps* ops) noexcept :
        Runtime(ops)
    {
    }

    ParserParameter(const ::TFParserParameterOps* ops, const ::TFParserParameter* handle) noexcept :
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
        const ice::sonic::ParserTypeInfo& out_typeinfo,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_typeinfo(get_handle(), out_typeinfo.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
