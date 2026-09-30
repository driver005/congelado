// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/function.h"

export module cc_ice_extern_parser_sonic:function;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFParserFunctionOps : public ice::sonic::Runtime<TFParserFunctionOps, TFParserFunctionOps>
{
public:
    explicit TFParserFunctionOps(TFParserFunctionOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "parser";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_name(const ice::sonic::String& out_name) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_name(get_handle(), out_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_parameter_count(int* out_count) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_parameter_count(get_handle(), out_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_parameter(int index, const ice::sonic::TFParserParameterOps& out_parameter) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_parameter(get_handle(), index, out_parameter.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_block_count(int* out_count) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_block_count(get_handle(), out_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_block(int index, const ice::sonic::TFParserBlockOps& out_block) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_block(get_handle(), index, out_block.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
