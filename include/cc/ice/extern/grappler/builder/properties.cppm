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
    static TFGrapplerPropertiesOps* create(void* ctx) noexcept
    {
        return static_cast<TFGrapplerPropertiesOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerPropertiesOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGrapplerPropertiesOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerPropertiesOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> infer_statically(
        _Bool assume_valid_feeds,
        _Bool aggressive_shape_inference,
        _Bool include_input_tensor_values,
        _Bool include_output_tensor_values
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_input_properties_size(const char* name, int* out_num_values) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_output_properties_size(const char* name, int* out_num_values) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_input_properties(const char* name, TF_Buffer** out_properties, int num_values) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_output_properties(
        const char* name,
        TF_Buffer** out_properties,
        int num_values
    ) noexcept = 0;

    static TFGrapplerPropertiesOps* get_generic_vtable()
    {
        static TFGrapplerPropertiesOps vtable = {
            .struct_size = TF_RAPPLERPROPERTIES_STRUCT_SIZE,
            .infer_statically =
                [](TFGrapplerProperties* props,
                   _Bool assume_valid_feeds,
                   _Bool aggressive_shape_inference,
                   _Bool include_input_tensor_values,
                   _Bool include_output_tensor_values,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerPropertiesOps::create(props);
                auto res = self->infer_statically(
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
                   const char* name,
                   int* out_num_values,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerPropertiesOps::create(props);
                auto res = self->get_input_properties_size(name, out_num_values);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_output_properties_size =
                [](TFGrapplerProperties* props,
                   const char* name,
                   int* out_num_values,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerPropertiesOps::create(props);
                auto res = self->get_output_properties_size(name, out_num_values);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_input_properties =
                [](TFGrapplerProperties* props,
                   const char* name,
                   TF_Buffer** out_properties,
                   int num_values,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerPropertiesOps::create(props);
                auto res = self->get_input_properties(name, out_properties, num_values);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_output_properties =
                [](TFGrapplerProperties* props,
                   const char* name,
                   TF_Buffer** out_properties,
                   int num_values,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerPropertiesOps::create(props);
                auto res = self->get_output_properties(name, out_properties, num_values);
                if (!res) {
                    res.error().to_c(out_status);
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
