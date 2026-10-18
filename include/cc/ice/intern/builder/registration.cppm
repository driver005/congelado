// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:registration;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Registration
{
public:
    explicit Registration(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    Registration(const Registration&) = delete;
    Registration& operator=(const Registration&) = delete;

    static Registration& from_handle(void* ctx) noexcept
    {
        return *static_cast<Registration*>(ctx);
    }

    template<typename HandleT>
    static Registration& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Registration*>(handle->plugin_data);
    }

    virtual ~Registration() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void register_op(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void* value
    ) noexcept = 0;
    virtual void
    get(const ice::sonic::String& type,
        const ice::sonic::String& name,
        void** out_value) noexcept = 0;
    virtual void
    unregister(const ice::sonic::String& type, const ice::sonic::String& name) noexcept = 0;
    virtual void
    set_default(const ice::sonic::String& type, const ice::sonic::String& name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Registration*)) noexcept
    {
        m_vtable = ::TF_RegistrationOps{
            .struct_size = TF_OFFSET_OF_END(::TF_RegistrationOps, set_default),

            .create = create,
            .destroy =
                [](::TF_Registration* handle) noexcept
            {
                auto& self = Registration::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TF_Registration* registration, ::TF_String* out_name) noexcept
            {
                auto& self = Registration::from_handle(registration);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .register_op =
                [](::TF_Registration* registration,
                   const ::TF_String* type,
                   const ::TF_String* name,
                   void* value) noexcept
            {
                auto& self = Registration::from_handle(registration);
                self.register_op(
                    self.wrap(std::type_identity<ice::sonic::String>{}, type),
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    value
                );
            },
            .get =
                [](const ::TF_Registration* registration,
                   const ::TF_String* type,
                   const ::TF_String* name,
                   void** out_value) noexcept
            {
                auto& self = Registration::from_handle(registration);
                self.get(
                    self.wrap(std::type_identity<ice::sonic::String>{}, type),
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_value
                );
            },
            .unregister =
                [](::TF_Registration* registration,
                   const ::TF_String* type,
                   const ::TF_String* name) noexcept
            {
                auto& self = Registration::from_handle(registration);
                self.unregister(
                    self.wrap(std::type_identity<ice::sonic::String>{}, type),
                    self.wrap(std::type_identity<ice::sonic::String>{}, name)
                );
            },
            .set_default =
                [](::TF_Registration* registration,
                   const ::TF_String* type,
                   const ::TF_String* name) noexcept
            {
                auto& self = Registration::from_handle(registration);
                self.set_default(
                    self.wrap(std::type_identity<ice::sonic::String>{}, type),
                    self.wrap(std::type_identity<ice::sonic::String>{}, name)
                );
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TF_RegistrationOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Registration& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_RegistrationOps*>(&m_vtable));
    }

private:
    ::TF_RegistrationOps m_vtable;
    ::TF_Registration m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
