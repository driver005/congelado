// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/catalog.h"

export module cc_ice_extern_parser_builder:catalog;

import std;

export namespace ice::builder {

class TFParserCatalogOps
{
public:
    static TFParserCatalogOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserCatalogOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserCatalogOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserCatalogOps*>(handle->plugin_data);
    }

    virtual ~TFParserCatalogOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> parse_file(
        const ice::sonic::TF_StringOps& file_path,
        const ice::sonic::TFParserModuleOps& out_module
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> parse_buffer(
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::TFParserModuleOps& out_module
    ) noexcept = 0;

    static TFParserCatalogOps* get_generic_vtable()
    {
        static TFParserCatalogOps vtable = {
            .struct_size = TF_ARSERCATALOG_STRUCT_SIZE,
            .parse_file =
                [](TFParserCatalog* catalog,
                   const TF_String* file_path,
                   TFParserModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserCatalogOps::create(catalog);
                auto res = self->parse_file(
                    ice::sonic::TF_StringOps::wrap(file_path),
                    ice::sonic::TFParserModuleOps::wrap(out_module)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .parse_buffer =
                [](TFParserCatalog* catalog,
                   const TF_Buffer* buffer,
                   TFParserModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserCatalogOps::create(catalog);
                auto res = self->parse_buffer(
                    ice::sonic::TF_BufferOps::wrap(buffer),
                    ice::sonic::TFParserModuleOps::wrap(out_module)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
