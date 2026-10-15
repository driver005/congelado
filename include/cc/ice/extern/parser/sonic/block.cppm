// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/block.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_parser_sonic:block;

import std;
import :node;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class ParserBlock : public ice::sonic::Runtime<::TFParserBlockOps, ::TFParserBlock>
{
public:
    ParserBlock(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    ParserBlock(
        const ice::sonic::Registration& registry,
        ::TFParserBlock* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit ParserBlock(const ::TFParserBlockOps* ops) noexcept :
        Runtime(ops)
    {
    }

    ParserBlock(const ::TFParserBlockOps* ops, ::TFParserBlock* handle) noexcept :
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

    void get_node_count(int& out_count, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_node_count(get_handle(), &out_count, out_status.get_handle());
    }

    void get_node(
        int index,
        const ice::sonic::ParserNode& out_node,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_node(get_handle(), index, out_node.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
