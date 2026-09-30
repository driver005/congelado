// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/registration/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_registration_builder:registration;

import std;

export namespace ice::builder {

class TF_RegistrationOps
{
public:
    TF_RegistrationOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_RegistrationOps(const TF_RegistrationOps&) = delete;
    TF_RegistrationOps& operator=(const TF_RegistrationOps&) = delete;

    static TF_RegistrationOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_RegistrationOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RegistrationOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_RegistrationOps*>(handle->plugin_data);
    }

    virtual ~TF_RegistrationOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_op(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void* value
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(const ice::sonic::String& type,
        const ice::sonic::String& name,
        void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    unregister(const ice::sonic::String& type, const ice::sonic::String& name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_RegistrationOps{
            .struct_size = TF_REGISTRATION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_RegistrationOps>{
                    &TF_RegistrationOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_RegistrationOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .register_op =
                [](TF_Registration* registration,
                   const TF_String* type,
                   const TF_String* name,
                   void* value) noexcept
            {
                auto res = TF_RegistrationOps::from_handle(registration)
                               .register_op(
                                   ice::sonic::String::wrap(type),
                                   ice::sonic::String::wrap(name),
                                   value
                               );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Registration* registration,
                   const TF_String* type,
                   const TF_String* name,
                   void** out_value) noexcept
            {
                auto res = TF_RegistrationOps::from_handle(registration)
                               .get(
                                   ice::sonic::String::wrap(type),
                                   ice::sonic::String::wrap(name),
                                   out_value
                               );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .unregister =
                [](TF_Registration* registration,
                   const TF_String* type,
                   const TF_String* name) noexcept
            {
                auto res =
                    TF_RegistrationOps::from_handle(registration)
                        .unregister(ice::sonic::String::wrap(type), ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_RegistrationOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Registration& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_RegistrationOps m_vtable;
    TF_Registration m_handle;
};

} // namespace ice::builder
