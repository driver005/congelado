// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/properties.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/properties.h"

export module cc_ice_extern_grappler_sonic:properties;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGrapplerPropertiesOps :
    public ice::sonic::Runtime<TFGrapplerPropertiesOps, TFGrapplerPropertiesOps>
{
public:
    explicit TFGrapplerPropertiesOps(TFGrapplerPropertiesOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    [[nodiscard]] std::expected<void, ice::sonic::Status> infer_statically(
        _Bool assume_valid_feeds,
        _Bool aggressive_shape_inference,
        _Bool include_input_tensor_values,
        _Bool include_output_tensor_values
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->infer_statically(
            get_handle(),
            assume_valid_feeds,
            aggressive_shape_inference,
            include_input_tensor_values,
            include_output_tensor_values,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_input_properties_size(const ice::sonic::String& name, int* out_num_values) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_input_properties_size(
            get_handle(),
            name.get_handle(),
            out_num_values,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_output_properties_size(const ice::sonic::String& name, int* out_num_values) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_output_properties_size(
            get_handle(),
            name.get_handle(),
            out_num_values,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_input_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_input_properties(
            get_handle(),
            name.get_handle(),
            out_properties,
            num_values,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_output_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_output_properties(
            get_handle(),
            name.get_handle(),
            out_properties,
            num_values,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
