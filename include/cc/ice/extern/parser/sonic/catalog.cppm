// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/catalog.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_parser_sonic:catalog;

import std;
import :module_;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class ParserCatalog : public ice::sonic::Runtime<::TFParserCatalogOps, ::TFParserCatalog>
{
public:
    ParserCatalog(
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

    ParserCatalog(
        const ice::sonic::Registration& registry,
        const ::TFParserCatalog& handle,
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

    explicit ParserCatalog(const ::TFParserCatalogOps* ops) noexcept :
        Runtime(ops)
    {
    }

    ParserCatalog(const ::TFParserCatalogOps* ops, const ::TFParserCatalog* handle) noexcept :
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

    void parse_file(
        const ice::sonic::String& file_path,
        const ice::sonic::ParserModule& out_module,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->parse_file(
            get_handle(),
            file_path.get_handle(),
            out_module.get_handle(),
            out_status.get_handle()
        );
    }

    void parse_buffer(
        const ice::sonic::Buffer& buffer,
        const ice::sonic::ParserModule& out_module,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->parse_buffer(
            get_handle(),
            buffer.get_handle(),
            out_module.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
