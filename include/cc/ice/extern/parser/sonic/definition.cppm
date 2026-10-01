// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/definition.h"

export module cc_ice_extern_parser_sonic:definition;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserDefinitionOps :
    public ice::sonic::Runtime<::TFParserDefinitionOps, ::TFParserDefinition>
{
public:
    template<typename Registry>
    TFParserDefinitionOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFParserDefinitionOps(
        Registry& registry,
        ::TFParserDefinition* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFParserDefinitionOps(const ::TFParserDefinitionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserDefinitionOps(const ::TFParserDefinitionOps* ops, ::TFParserDefinition* handle) noexcept
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

    void get_name(
        const ice::sonic::String& out_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle(), out_status.get_handle());
    }

    void get_source_file(
        const ice::sonic::String& out_source_file,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_source_file(get_handle(), out_source_file.get_handle(), out_status.get_handle());
    }

    void get_line_number(int* out_line_number, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_line_number(get_handle(), out_line_number, out_status.get_handle());
    }
};

} // namespace ice::sonic
