// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/properties.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/properties.h"

export module cc_ice_extern_grappler_sonic:properties;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGrapplerPropertiesOps :
    public ice::sonic::Runtime<::TFGrapplerPropertiesOps, ::TFGrapplerProperties>
{
public:
    template<typename Registry>
    TFGrapplerPropertiesOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFGrapplerPropertiesOps(
        Registry& registry,
        ::TFGrapplerProperties* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFGrapplerPropertiesOps(const ::TFGrapplerPropertiesOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGrapplerPropertiesOps(
        const ::TFGrapplerPropertiesOps* ops,
        ::TFGrapplerProperties* handle
    ) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void infer_statically(
        _Bool assume_valid_feeds,
        _Bool aggressive_shape_inference,
        _Bool include_input_tensor_values,
        _Bool include_output_tensor_values,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->infer_statically(
            get_handle(),
            assume_valid_feeds,
            aggressive_shape_inference,
            include_input_tensor_values,
            include_output_tensor_values,
            out_status.get_handle()
        );
    }

    void get_input_properties_size(
        const ice::sonic::String& name,
        int* out_num_values,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_input_properties_size(
            get_handle(),
            name.get_handle(),
            out_num_values,
            out_status.get_handle()
        );
    }

    void get_output_properties_size(
        const ice::sonic::String& name,
        int* out_num_values,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_output_properties_size(
            get_handle(),
            name.get_handle(),
            out_num_values,
            out_status.get_handle()
        );
    }

    void get_input_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_input_properties(
            get_handle(),
            name.get_handle(),
            out_properties,
            num_values,
            out_status.get_handle()
        );
    }

    void get_output_properties(
        const ice::sonic::String& name,
        TF_Buffer** out_properties,
        int num_values,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_output_properties(
            get_handle(),
            name.get_handle(),
            out_properties,
            num_values,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
