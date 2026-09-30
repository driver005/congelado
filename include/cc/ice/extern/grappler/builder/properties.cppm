// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/properties.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/properties.h"

export module cc_ice_extern_grappler_builder:properties;

import std;

export namespace ice::builder {

class TFGrapplerPropertiesOps
{
public:
    TFGrapplerPropertiesOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> infer_statically(
        _Bool assume_valid_feeds,
        _Bool aggressive_shape_inference,
        _Bool include_input_tensor_values,
        _Bool include_output_tensor_values
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_input_properties_size(const ice::sonic::String& name, int* out_num_values) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_output_properties_size(const ice::sonic::String& name, int* out_num_values) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_input_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_output_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGrapplerPropertiesOps{
            .struct_size = TF_RAPPLERPROPERTIES_STRUCT_SIZE,
            .infer_statically =
                [](TFGrapplerProperties* props,
                   _Bool assume_valid_feeds,
                   _Bool aggressive_shape_inference,
                   _Bool include_input_tensor_values,
                   _Bool include_output_tensor_values,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerPropertiesOps::from_handle(props).infer_statically(
                    assume_valid_feeds,
                    aggressive_shape_inference,
                    include_input_tensor_values,
                    include_output_tensor_values
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_input_properties_size =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   int* out_num_values,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerPropertiesOps::from_handle(props).get_input_properties_size(
                    ice::sonic::String::wrap(name),
                    out_num_values
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_output_properties_size =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   int* out_num_values,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerPropertiesOps::from_handle(props).get_output_properties_size(
                    ice::sonic::String::wrap(name),
                    out_num_values
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_input_properties =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   TF_Buffer** out_properties,
                   int num_values,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerPropertiesOps::from_handle(props).get_input_properties(
                    ice::sonic::String::wrap(name),
                    out_properties,
                    num_values
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_output_properties =
                [](TFGrapplerProperties* props,
                   const TF_String* name,
                   TF_Buffer** out_properties,
                   int num_values,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerPropertiesOps::from_handle(props).get_output_properties(
                    ice::sonic::String::wrap(name),
                    out_properties,
                    num_values
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGrapplerPropertiesOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGrapplerProperties& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGrapplerPropertiesOps m_vtable;
    TFGrapplerProperties m_handle;
};

} // namespace ice::builder
