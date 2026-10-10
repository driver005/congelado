// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/definition.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:definition;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserDefinitionOps
{
public:
    explicit TFParserDefinitionOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFParserDefinitionOps(const TFParserDefinitionOps&) = delete;
    TFParserDefinitionOps& operator=(const TFParserDefinitionOps&) = delete;

    static TFParserDefinitionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserDefinitionOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserDefinitionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserDefinitionOps*>(handle->plugin_data);
    }

    virtual ~TFParserDefinitionOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_source_file(
        const ice::sonic::String& out_source_file,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_line_number(int* out_line_number, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserDefinition*)) noexcept
    {
        m_vtable = ::TFParserDefinitionOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserDefinitionOps, get_line_number),

            .create = create,
            .destroy =
                [](TFParserDefinition* handle) noexcept
            {
                auto& self = TFParserDefinitionOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFParserDefinition* definition,
                   TF_String* out_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserDefinitionOps::from_handle(definition);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_source_file =
                [](TFParserDefinition* definition,
                   TF_String* out_source_file,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserDefinitionOps::from_handle(definition);
                self.get_source_file(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_source_file),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_line_number =
                [](TFParserDefinition* definition,
                   int* out_line_number,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserDefinitionOps::from_handle(definition);
                self.get_line_number(
                    out_line_number,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
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

    const ::TFParserDefinitionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserDefinition& get_handle() const noexcept
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
            const_cast<::TFParserDefinitionOps*>(&m_vtable)
        );
    }

private:
    ::TFParserDefinitionOps m_vtable;
    ::TFParserDefinition m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
