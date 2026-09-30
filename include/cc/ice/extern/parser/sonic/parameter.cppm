// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parameter.h"

export module cc_ice_extern_parser_sonic:parameter;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFParserParameterOps : public ice::sonic::Runtime<TFParserParameterOps, TFParserParameterOps>
{
public:
    explicit TFParserParameterOps(TFParserParameterOps* ops, void* plugin_context) noexcept :
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
    get_typeinfo(const ice::sonic::TFParserTypeInfoOps& out_typeinfo) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_typeinfo(get_handle(), out_typeinfo.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
