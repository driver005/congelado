// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/serde/serde.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_serde_builder:serde;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_SerdeOps
{
public:
    explicit TF_SerdeOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_SerdeOps(const TF_SerdeOps&) = delete;
    TF_SerdeOps& operator=(const TF_SerdeOps&) = delete;

    static TF_SerdeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SerdeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SerdeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SerdeOps*>(handle->plugin_data);
    }

    virtual ~TF_SerdeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_content_type(const ice::sonic::String& out_content_type) noexcept = 0;
    virtual void get_format_name(const ice::sonic::String& out_format_name) noexcept = 0;
    virtual void encode(
        const ice::sonic::String& value_json,
        const ice::sonic::String& out_encoded,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void decode(
        const ice::sonic::String& data,
        const ice::sonic::String& out_json,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Serde*)) noexcept
    {
        m_vtable = ::TF_SerdeOps{
            .struct_size = TF_OFFSET_OF_END(::TF_SerdeOps, decode),

            .create = create,
            .destroy =
                [](TF_Serde* handle) noexcept
            {
                auto& self = TF_SerdeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Serde* serde, TF_String* out_name) noexcept
            {
                auto& self = TF_SerdeOps::from_handle(serde);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .get_content_type =
                [](TF_Serde* serde, TF_String* out_content_type) noexcept
            {
                auto& self = TF_SerdeOps::from_handle(serde);
                self.get_content_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_content_type)
                );
            },
            .get_format_name =
                [](TF_Serde* serde, TF_String* out_format_name) noexcept
            {
                auto& self = TF_SerdeOps::from_handle(serde);
                self.get_format_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_format_name)
                );
            },
            .encode =
                [](TF_Serde* serde,
                   const TF_String* value_json,
                   TF_String* out_encoded,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SerdeOps::from_handle(serde);
                self.encode(
                    self.wrap(std::type_identity<ice::sonic::String>{}, value_json),
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_encoded),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .decode =
                [](TF_Serde* serde,
                   const TF_String* data,
                   TF_String* out_json,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SerdeOps::from_handle(serde);
                self.decode(
                    self.wrap(std::type_identity<ice::sonic::String>{}, data),
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_json),
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

    const ::TF_SerdeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Serde& get_handle() const noexcept
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
            const_cast<::TF_SerdeOps*>(&m_vtable)
        );
    }

private:
    ::TF_SerdeOps m_vtable;
    ::TF_Serde m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
