// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:attribute;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGeneratorAttributeOps
{
public:
    explicit TFGeneratorAttributeOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TFGeneratorAttributeOps(const TFGeneratorAttributeOps&) = delete;
    TFGeneratorAttributeOps& operator=(const TFGeneratorAttributeOps&) = delete;

    static TFGeneratorAttributeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorAttributeOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorAttributeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorAttributeOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorAttributeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;
    virtual void set_description(const ice::sonic::String& description) noexcept = 0;
    virtual void set_full_type(const ice::sonic::String& full_type) noexcept = 0;
    virtual void set_base_type(const ice::sonic::String& base_type) noexcept = 0;
    virtual void set_is_list(_Bool is_list) noexcept = 0;
    virtual void get_description(const ice::sonic::String& out_description) noexcept = 0;
    virtual void get_full_type(const ice::sonic::String& out_full_type) noexcept = 0;
    virtual void get_base_type(const ice::sonic::String& out_base_type) noexcept = 0;
    virtual void is_list(int* out_is_list) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorAttribute*)) noexcept
    {
        m_vtable = ::TFGeneratorAttributeOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorAttributeOps, is_list),

            .create = create,
            .destroy =
                [](TFGeneratorAttribute* handle) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFGeneratorAttribute* attr_context, TF_String* out_name) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_name =
                [](TFGeneratorAttribute* attr_context, const TF_String* name) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.set_name(self.wrap(std::type_identity<ice::sonic::String>{}, name));
            },
            .set_description =
                [](TFGeneratorAttribute* attr_context, const TF_String* description) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.set_description(
                    self.wrap(std::type_identity<ice::sonic::String>{}, description)
                );
            },
            .set_full_type =
                [](TFGeneratorAttribute* attr_context, const TF_String* full_type) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.set_full_type(self.wrap(std::type_identity<ice::sonic::String>{}, full_type));
            },
            .set_base_type =
                [](TFGeneratorAttribute* attr_context, const TF_String* base_type) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.set_base_type(self.wrap(std::type_identity<ice::sonic::String>{}, base_type));
            },
            .set_is_list =
                [](TFGeneratorAttribute* attr_context, _Bool is_list) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.set_is_list(is_list);
            },
            .get_description =
                [](TFGeneratorAttribute* attr_context, TF_String* out_description) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.get_description(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_description)
                );
            },
            .get_full_type =
                [](TFGeneratorAttribute* attr_context, TF_String* out_full_type) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.get_full_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_full_type)
                );
            },
            .get_base_type =
                [](TFGeneratorAttribute* attr_context, TF_String* out_base_type) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.get_base_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_base_type)
                );
            },
            .is_list =
                [](TFGeneratorAttribute* attr_context, int* out_is_list) noexcept
            {
                auto& self = TFGeneratorAttributeOps::from_handle(attr_context);
                self.is_list(out_is_list);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFGeneratorAttributeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorAttribute& get_handle() const noexcept
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
            const_cast<::TFGeneratorAttributeOps*>(&m_vtable)
        );
    }

private:
    ::TFGeneratorAttributeOps m_vtable;
    ::TFGeneratorAttribute m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
