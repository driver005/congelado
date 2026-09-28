// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/module.h"

export module cc_ice_extern_parser_sonic:module_;

import std;
import cc_ice_support;
import c_intern;
import c_intern;
import c_intern;
import :function;
import c_intern;
import c_intern;
import :parameter;
import c_intern;
import c_intern;
import :typeinfo;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import :block;
import c_intern;
import c_intern;
import :node;
import c_intern;
import c_intern;
import :attribute;
import c_intern;
import c_intern;
import c_intern;
import :definition;
import c_intern;
import c_intern;

export namespace ice::sonic {

class TFParserModuleOps :
    public ice::sonic::Runtime<TFParserModuleOps, ::TFParserModuleOps, ::TFParserModule>
{
public:
    explicit TFParserModuleOps(::TFParserModuleOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "parser";

    [[nodiscard]] std::expected<void, TF_Status>
    get_name(const ice::sonic::TF_StringOps& out_name) noexcept
    {
        TF_Status status;
        m_ops->get_name(get_handle(), out_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, TF_Status> get_function_count(int* out_count) noexcept
    {
        TF_Status status;
        m_ops->get_function_count(get_handle(), out_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, TF_Status>
    get_function(int index, const ice::sonic::TFParserFunctionOps& out_function) noexcept
    {
        TF_Status status;
        m_ops->get_function(get_handle(), index, out_function.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
