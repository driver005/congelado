// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/registration/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"

export module cc_ice_builder_registration:registration;

import std;

export namespace ice::builder {

class TF_RegistrationOps
{
public:
    static TF_RegistrationOps* create(void* ctx) noexcept
    {
        return static_cast<TF_RegistrationOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RegistrationOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_RegistrationOps*>(handle->plugin_data);
    }

    virtual ~TF_RegistrationOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_op(
        const ice::sonic::TF_StringOps& type,
        const ice::sonic::TF_StringOps& name,
        void* value
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(const ice::sonic::TF_StringOps& type,
        const ice::sonic::TF_StringOps& name,
        void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> unregister(
        const ice::sonic::TF_StringOps& type,
        const ice::sonic::TF_StringOps& name
    ) noexcept = 0;

    static TF_RegistrationOps* get_generic_vtable()
    {
        static TF_RegistrationOps vtable = {
            .struct_size = TF_REGISTRATION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_RegistrationOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_RegistrationOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .register_op =
                [](TF_Registration* registration,
                   const TF_String* type,
                   const TF_String* name,
                   void* value) noexcept
            {
                auto* self = TF_RegistrationOps::create(registration);
                auto res = self->register_op(
                    ice::sonic::TF_StringOps::wrap(type),
                    ice::sonic::TF_StringOps::wrap(name),
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
                auto* self = TF_RegistrationOps::create(registration);
                auto res = self->get(
                    ice::sonic::TF_StringOps::wrap(type),
                    ice::sonic::TF_StringOps::wrap(name),
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
                auto* self = TF_RegistrationOps::create(registration);
                auto res = self->unregister(
                    ice::sonic::TF_StringOps::wrap(type),
                    ice::sonic::TF_StringOps::wrap(name)
                );
                if (!res) {
                    res.error().to_c(status);
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
