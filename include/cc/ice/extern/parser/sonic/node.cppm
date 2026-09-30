// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/node.h"

export module cc_ice_extern_parser_sonic:node;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFParserNodeOps : public ice::sonic::Runtime<TFParserNodeOps, TFParserNodeOps>
{
public:
    explicit TFParserNodeOps(TFParserNodeOps* ops, void* plugin_context) noexcept :
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
    get_op_type(const ice::sonic::String& out_op_type) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_op_type(get_handle(), out_op_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_attribute_count(int* out_count) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_attribute_count(get_handle(), out_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_attribute(int index, const ice::sonic::TFParserAttributeOps& out_attribute) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_attribute(get_handle(), index, out_attribute.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_definition(const ice::sonic::TFParserDefinitionOps& out_definition) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_definition(get_handle(), out_definition.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
