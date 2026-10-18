// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/catalog.h"
#include "include/c/extern/parser/module.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:catalog;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class ParserCatalog
{
public:
    explicit ParserCatalog(
        const ::TFParserModuleOps* ParserModule_ops,
        const ::TF_BufferOps* Buffer_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_ParserModule_ops = ParserModule_ops;
        m_Buffer_ops = Buffer_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    ParserCatalog(const ParserCatalog&) = delete;
    ParserCatalog& operator=(const ParserCatalog&) = delete;

    static ParserCatalog& from_handle(void* ctx) noexcept
    {
        return *static_cast<ParserCatalog*>(ctx);
    }

    template<typename HandleT>
    static ParserCatalog& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<ParserCatalog*>(handle->plugin_data);
    }

    virtual ~ParserCatalog() = default;
    virtual void destroy() noexcept = 0;
    virtual void parse_file(
        const ice::sonic::String& file_path,
        const ice::sonic::ParserModule& out_module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void parse_buffer(
        const ice::sonic::Buffer& buffer,
        const ice::sonic::ParserModule& out_module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserCatalog*)) noexcept
    {
        m_vtable = ::TFParserCatalogOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserCatalogOps, parse_buffer),

            .create = create,
            .destroy =
                [](::TFParserCatalog* handle) noexcept
            {
                auto& self = ParserCatalog::from_handle(handle);
                self.destroy();
            },
            .parse_file =
                [](::TFParserCatalog* catalog,
                   const ::TF_String* file_path,
                   ::TFParserModule* out_module,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserCatalog::from_handle(catalog);
                self.parse_file(
                    self.wrap(std::type_identity<ice::sonic::String>{}, file_path),
                    self.wrap(std::type_identity<ice::sonic::ParserModule>{}, out_module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .parse_buffer =
                [](::TFParserCatalog* catalog,
                   const ::TF_Buffer* buffer,
                   ::TFParserModule* out_module,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserCatalog::from_handle(catalog);
                self.parse_buffer(
                    self.wrap(std::type_identity<ice::sonic::Buffer>{}, buffer),
                    self.wrap(std::type_identity<ice::sonic::ParserModule>{}, out_module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::ParserModule wrap(
        std::type_identity<ice::sonic::ParserModule>,
        const ::TFParserModule* handle
    ) const noexcept
    {
        return ice::sonic::ParserModule{m_ParserModule_ops, handle};
    }

    ice::sonic::Buffer
    wrap(std::type_identity<ice::sonic::Buffer>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::Buffer{m_Buffer_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFParserCatalogOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserCatalog& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFParserCatalogOps*>(&m_vtable));
    }

private:
    ::TFParserCatalogOps m_vtable;
    ::TFParserCatalog m_handle;

    const ::TFParserModuleOps* m_ParserModule_ops{nullptr};

    const ::TF_BufferOps* m_Buffer_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
