// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/catalog.h"
#include "include/c/extern/parser/module.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:catalog;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserCatalogOps
{
public:
    explicit TFParserCatalogOps(
        const ::TFParserModuleOps* TFParserModuleOps_ops,
        const ::TF_BufferOps* TF_BufferOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFParserModuleOps_ops = TFParserModuleOps_ops;
        m_TF_BufferOps_ops = TF_BufferOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFParserCatalogOps(const TFParserCatalogOps&) = delete;
    TFParserCatalogOps& operator=(const TFParserCatalogOps&) = delete;

    static TFParserCatalogOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserCatalogOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserCatalogOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserCatalogOps*>(handle->plugin_data);
    }

    virtual ~TFParserCatalogOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void parse_file(
        const ice::sonic::String& file_path,
        const ice::sonic::TFParserModuleOps& out_module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void parse_buffer(
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::TFParserModuleOps& out_module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserCatalog*)) noexcept
    {
        m_vtable = ::TFParserCatalogOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserCatalogOps, parse_buffer),

            .create = create,
            .destroy =
                [](TFParserCatalog* handle) noexcept
            {
                auto& self = TFParserCatalogOps::from_handle(handle);
                self.destroy();
            },
            .parse_file =
                [](TFParserCatalog* catalog,
                   const TF_String* file_path,
                   TFParserModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserCatalogOps::from_handle(catalog);
                self.parse_file(
                    self.wrap(std::type_identity<ice::sonic::String>{}, file_path),
                    self.wrap(std::type_identity<ice::sonic::TFParserModuleOps>{}, out_module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .parse_buffer =
                [](TFParserCatalog* catalog,
                   const TF_Buffer* buffer,
                   TFParserModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserCatalogOps::from_handle(catalog);
                self.parse_buffer(
                    self.wrap(std::type_identity<ice::sonic::TF_BufferOps>{}, buffer),
                    self.wrap(std::type_identity<ice::sonic::TFParserModuleOps>{}, out_module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFParserModuleOps wrap(
        std::type_identity<ice::sonic::TFParserModuleOps>,
        const ::TFParserModule* handle
    ) const noexcept
    {
        return ice::sonic::TFParserModuleOps{
            m_TFParserModuleOps_ops,
            const_cast<::TFParserModule*>(handle)
        };
    }

    ice::sonic::TF_BufferOps
    wrap(std::type_identity<ice::sonic::TF_BufferOps>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::TF_BufferOps{m_TF_BufferOps_ops, const_cast<::TF_Buffer*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
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
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFParserCatalogOps*>(&m_vtable)
        );
    }

private:
    ::TFParserCatalogOps m_vtable;
    ::TFParserCatalog m_handle;

    const ::TFParserModuleOps* m_TFParserModuleOps_ops{nullptr};

    const ::TF_BufferOps* m_TF_BufferOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
