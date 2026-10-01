// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/function.h"

export module cc_ice_extern_parser_sonic:function;

import std;
import :block;
import :parameter;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserFunctionOps : public ice::sonic::Runtime<::TFParserFunctionOps, ::TFParserFunction>
{
public:
    template<typename Registry>
    TFParserFunctionOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFParserFunctionOps(
        Registry& registry,
        ::TFParserFunction* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFParserFunctionOps(const ::TFParserFunctionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserFunctionOps(const ::TFParserFunctionOps* ops, ::TFParserFunction* handle) noexcept :
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

    void get_parameter_count(int* out_count, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_parameter_count(get_handle(), out_count, out_status.get_handle());
    }

    void get_parameter(
        int index,
        const ice::sonic::TFParserParameterOps& out_parameter,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_parameter(
            get_handle(),
            index,
            out_parameter.get_handle(),
            out_status.get_handle()
        );
    }

    void get_block_count(int* out_count, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_block_count(get_handle(), out_count, out_status.get_handle());
    }

    void get_block(
        int index,
        const ice::sonic::TFParserBlockOps& out_block,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_block(get_handle(), index, out_block.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
