// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parameter.h"
#include "include/c/extern/parser/typeinfo.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:parameter;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserParameterOps
{
public:
    explicit TFParserParameterOps(
        const ::TFParserTypeInfoOps* TFParserTypeInfoOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFParserTypeInfoOps_ops = TFParserTypeInfoOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFParserParameterOps(const TFParserParameterOps&) = delete;
    TFParserParameterOps& operator=(const TFParserParameterOps&) = delete;

    static TFParserParameterOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserParameterOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserParameterOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserParameterOps*>(handle->plugin_data);
    }

    virtual ~TFParserParameterOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_typeinfo(
        const ice::sonic::TFParserTypeInfoOps& out_typeinfo,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserParameter*)) noexcept
    {
        m_vtable = ::TFParserParameterOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserParameterOps, get_typeinfo),

            .create = create,
            .destroy =
                [](TFParserParameter* handle) noexcept
            {
                auto& self = TFParserParameterOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFParserParameter* parameter,
                   TF_String* out_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserParameterOps::from_handle(parameter);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_typeinfo =
                [](TFParserParameter* parameter,
                   TFParserTypeInfo* out_typeinfo,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserParameterOps::from_handle(parameter);
                self.get_typeinfo(
                    self.wrap(std::type_identity<ice::sonic::TFParserTypeInfoOps>{}, out_typeinfo),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFParserTypeInfoOps wrap(
        std::type_identity<ice::sonic::TFParserTypeInfoOps>,
        const ::TFParserTypeInfo* handle
    ) const noexcept
    {
        return ice::sonic::TFParserTypeInfoOps{
            m_TFParserTypeInfoOps_ops,
            const_cast<::TFParserTypeInfo*>(handle)
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

    const ::TFParserParameterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserParameter& get_handle() const noexcept
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
            const_cast<::TFParserParameterOps*>(&m_vtable)
        );
    }

private:
    ::TFParserParameterOps m_vtable;
    ::TFParserParameter m_handle;

    const ::TFParserTypeInfoOps* m_TFParserTypeInfoOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
