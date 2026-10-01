// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/construction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/construction.h"

export module cc_ice_extern_kernel_sonic:construction;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_OpKernelConstructionOps :
    public ice::sonic::Runtime<::TF_OpKernelConstructionOps, ::TF_OpKernelConstruction>
{
public:
    template<typename Registry>
    TF_OpKernelConstructionOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_OpKernelConstructionOps(
        Registry& registry,
        ::TF_OpKernelConstruction* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_OpKernelConstructionOps(const ::TF_OpKernelConstructionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_OpKernelConstructionOps(
        const ::TF_OpKernelConstructionOps* ops,
        ::TF_OpKernelConstruction* handle
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

    void failure(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->failure(get_handle(), out_status.get_handle());
    }

    void get_node_def(
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_node_def(get_handle(), buffer.get_handle(), out_status.get_handle());
    }

    void get_attr_size(
        const ice::sonic::String& attr_name,
        int32_t* out_list_size,
        int32_t* out_total_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_size(
            get_handle(),
            attr_name.get_handle(),
            out_list_size,
            out_total_size,
            out_status.get_handle()
        );
    }

    void get_attr_type(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->get_attr_type(get_handle(), attr_name.get_handle(), out_val, out_status.get_handle());
    }

    void get_attr_int32(
        const ice::sonic::String& attr_name,
        int32_t* out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_int32(
            get_handle(),
            attr_name.get_handle(),
            out_val,
            out_status.get_handle()
        );
    }

    void get_attr_int64(
        const ice::sonic::String& attr_name,
        int64_t* out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_int64(
            get_handle(),
            attr_name.get_handle(),
            out_val,
            out_status.get_handle()
        );
    }

    void get_attr_float(
        const ice::sonic::String& attr_name,
        float* out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_float(
            get_handle(),
            attr_name.get_handle(),
            out_val,
            out_status.get_handle()
        );
    }

    void get_attr_bool(
        const ice::sonic::String& attr_name,
        _Bool* out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->get_attr_bool(get_handle(), attr_name.get_handle(), out_val, out_status.get_handle());
    }

    void get_attr_string(
        const ice::sonic::String& attr_name,
        const ice::sonic::String& out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_string(
            get_handle(),
            attr_name.get_handle(),
            out_val.get_handle(),
            out_status.get_handle()
        );
    }

    void get_attr_tensor(
        const ice::sonic::String& attr_name,
        TF_Tensor** out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_tensor(
            get_handle(),
            attr_name.get_handle(),
            out_val,
            out_status.get_handle()
        );
    }

    void get_attr_type_list(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_type_list(
            get_handle(),
            attr_name.get_handle(),
            out_vals,
            max_vals,
            out_status.get_handle()
        );
    }

    void get_attr_int32_list(
        const ice::sonic::String& attr_name,
        int32_t* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_int32_list(
            get_handle(),
            attr_name.get_handle(),
            out_vals,
            max_vals,
            out_status.get_handle()
        );
    }

    void get_attr_int64_list(
        const ice::sonic::String& attr_name,
        int64_t* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_int64_list(
            get_handle(),
            attr_name.get_handle(),
            out_vals,
            max_vals,
            out_status.get_handle()
        );
    }

    void get_attr_float_list(
        const ice::sonic::String& attr_name,
        float* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_float_list(
            get_handle(),
            attr_name.get_handle(),
            out_vals,
            max_vals,
            out_status.get_handle()
        );
    }

    void get_attr_bool_list(
        const ice::sonic::String& attr_name,
        _Bool* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_bool_list(
            get_handle(),
            attr_name.get_handle(),
            out_vals,
            max_vals,
            out_status.get_handle()
        );
    }

    void get_attr_string_list(
        const ice::sonic::String& attr_name,
        char** out_values,
        size_t* out_lengths,
        int max_values,
        void* storage,
        size_t storage_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_string_list(
            get_handle(),
            attr_name.get_handle(),
            out_values,
            out_lengths,
            max_values,
            storage,
            storage_size,
            out_status.get_handle()
        );
    }

    void get_attr_tensor_list(
        const ice::sonic::String& attr_name,
        TF_Tensor** out_vals,
        int max_values,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_tensor_list(
            get_handle(),
            attr_name.get_handle(),
            out_vals,
            max_values,
            out_status.get_handle()
        );
    }

    void get_attr_function(
        const ice::sonic::String& attr_name,
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_function(
            get_handle(),
            attr_name.get_handle(),
            buffer.get_handle(),
            out_status.get_handle()
        );
    }

    void has_attr(
        const ice::sonic::String& attr_name,
        _Bool* out_has_attr,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->has_attr(get_handle(), attr_name.get_handle(), out_has_attr, out_status.get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void get_attr_tensor_shape(
        const ice::sonic::String& attr_name,
        int64_t* out_dims,
        size_t num_dims,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attr_tensor_shape(
            get_handle(),
            attr_name.get_handle(),
            out_dims,
            num_dims,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
