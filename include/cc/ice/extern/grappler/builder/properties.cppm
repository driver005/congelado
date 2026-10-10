// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/properties.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/properties.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_grappler_builder:properties;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGrapplerPropertiesOps
{
public:
    explicit TFGrapplerPropertiesOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFGrapplerPropertiesOps(const TFGrapplerPropertiesOps&) = delete;
    TFGrapplerPropertiesOps& operator=(const TFGrapplerPropertiesOps&) = delete;

    static TFGrapplerPropertiesOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerPropertiesOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerPropertiesOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerPropertiesOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerPropertiesOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void infer_statically(
        _Bool assume_valid_feeds,
        _Bool aggressive_shape_inference,
        _Bool include_input_tensor_values,
        _Bool include_output_tensor_values,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_input_properties_size(
        const ice::sonic::String& name,
        int* out_num_values,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_output_properties_size(
        const ice::sonic::String& name,
        int* out_num_values,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_input_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_output_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerProperties*)) noexcept
    {
        m_vtable = ::TFGrapplerPropertiesOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerPropertiesOps, get_output_properties),

            .create = create,
            .destroy =
                [](TFGrapplerProperties* handle) noexcept
            {
                auto& self = TFGrapplerPropertiesOps::from_handle(handle);
                self.destroy();
            },
            .infer_statically =
                [](TFGrapplerProperties* props,
                   _Bool assume_valid_feeds,
                   _Bool aggressive_shape_inference,
                   _Bool include_input_tensor_values,
                   _Bool include_output_tensor_values,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerPropertiesOps::from_handle(props);
                self.infer_statically(
                    assume_valid_feeds,
                    aggressive_shape_inference,
                    include_input_tensor_values,
                    include_output_tensor_values,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_input_properties_size =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   int* out_num_values,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerPropertiesOps::from_handle(props);
                self.get_input_properties_size(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_num_values,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_output_properties_size =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   int* out_num_values,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerPropertiesOps::from_handle(props);
                self.get_output_properties_size(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_num_values,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_input_properties =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   TF_Buffer** out_properties,
                   int num_values,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerPropertiesOps::from_handle(props);
                self.get_input_properties(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_properties,
                    num_values,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_output_properties =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   TF_Buffer** out_properties,
                   int num_values,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerPropertiesOps::from_handle(props);
                self.get_output_properties(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_properties,
                    num_values,
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

    const ::TFGrapplerPropertiesOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerProperties& get_handle() const noexcept
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
            const_cast<::TFGrapplerPropertiesOps*>(&m_vtable)
        );
    }

private:
    ::TFGrapplerPropertiesOps m_vtable;
    ::TFGrapplerProperties m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
