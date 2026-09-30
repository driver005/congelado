// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/catalog.h"

export module cc_ice_extern_parser_sonic:catalog;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFParserCatalogOps : public ice::sonic::Runtime<TFParserCatalogOps, TFParserCatalogOps>
{
public:
    explicit TFParserCatalogOps(TFParserCatalogOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "parser";

    [[nodiscard]] std::expected<void, ice::sonic::Status> parse_file(
        const ice::sonic::String& file_path,
        const ice::sonic::TFParserModuleOps& out_module
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->parse_file(
            get_handle(),
            file_path.get_handle(),
            out_module.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> parse_buffer(
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::TFParserModuleOps& out_module
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->parse_buffer(
            get_handle(),
            buffer.get_handle(),
            out_module.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
