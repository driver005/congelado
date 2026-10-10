// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/parameter.h"
#include "include/c/extern/generator/typeinfo.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:parameter;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGeneratorParameterOps
{
public:
    explicit TFGeneratorParameterOps(
        const ::TF_StringOps* String_ops,
        const ::TF_TypeInfoOps* TF_TypeInfoOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
        m_TF_TypeInfoOps_ops = TF_TypeInfoOps_ops;
    }

    TFGeneratorParameterOps(const TFGeneratorParameterOps&) = delete;
    TFGeneratorParameterOps& operator=(const TFGeneratorParameterOps&) = delete;

    static TFGeneratorParameterOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorParameterOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorParameterOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorParameterOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorParameterOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;
    virtual void set_description(const ice::sonic::String& description) noexcept = 0;
    virtual void set_position(int position) noexcept = 0;
    virtual void get_description(const ice::sonic::String& out_description) noexcept = 0;
    virtual void get_position(int* out_position) noexcept = 0;
    virtual void get_type(const ice::sonic::TF_TypeInfoOps& out_type) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorParameter*)) noexcept
    {
        m_vtable = ::TFGeneratorParameterOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorParameterOps, get_type),

            .create = create,
            .destroy =
                [](TFGeneratorParameter* handle) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFGeneratorParameter* param_context, TF_String* out_name) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_name =
                [](TFGeneratorParameter* param_context, const TF_String* name) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.set_name(self.wrap(std::type_identity<ice::sonic::String>{}, name));
            },
            .set_description =
                [](TFGeneratorParameter* param_context, const TF_String* description) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.set_description(
                    self.wrap(std::type_identity<ice::sonic::String>{}, description)
                );
            },
            .set_position =
                [](TFGeneratorParameter* param_context, int position) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.set_position(position);
            },
            .get_description =
                [](TFGeneratorParameter* param_context, TF_String* out_description) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.get_description(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_description)
                );
            },
            .get_position =
                [](TFGeneratorParameter* param_context, int* out_position) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.get_position(out_position);
            },
            .get_type =
                [](TFGeneratorParameter* param_context, TF_TypeInfo* out_type) noexcept
            {
                auto& self = TFGeneratorParameterOps::from_handle(param_context);
                self.get_type(
                    self.wrap(std::type_identity<ice::sonic::TF_TypeInfoOps>{}, out_type)
                );
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_TypeInfoOps
    wrap(std::type_identity<ice::sonic::TF_TypeInfoOps>, const ::TF_TypeInfo* handle) const noexcept
    {
        return ice::sonic::TF_TypeInfoOps{m_TF_TypeInfoOps_ops, const_cast<::TF_TypeInfo*>(handle)};
    }

    const ::TFGeneratorParameterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorParameter& get_handle() const noexcept
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
            const_cast<::TFGeneratorParameterOps*>(&m_vtable)
        );
    }

private:
    ::TFGeneratorParameterOps m_vtable;
    ::TFGeneratorParameter m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_TypeInfoOps* m_TF_TypeInfoOps_ops{nullptr};
};

} // namespace ice::builder
