// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tensor.h"

export module cc_ice_intern_sonic:tensor;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TensorOps : public ice::sonic::Runtime<TF_TensorOps, TF_TensorOps>
{
public:
    explicit TF_TensorOps(TF_TensorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_dtype(TFDataTypeEnum dtype) noexcept
    {
        m_ops->set_dtype(get_handle(), dtype);
    }

    void set_dims(const int64_t* dims, int num_dims) noexcept
    {
        m_ops->set_dims(get_handle(), dims, num_dims);
    }

    void set_byte_size(size_t len) noexcept
    {
        m_ops->set_byte_size(get_handle(), len);
    }

    void delete_tensor() noexcept
    {
        m_ops->delete_tensor(get_handle());
    }

    void tensor_type(TFDataTypeEnum* out_dtype) noexcept
    {
        m_ops->tensor_type(get_handle(), out_dtype);
    }

    void num_dims(int* out_num_dims) noexcept
    {
        m_ops->num_dims(get_handle(), out_num_dims);
    }

    void dim(int dim_index, int64_t* out_dim) noexcept
    {
        m_ops->dim(get_handle(), dim_index, out_dim);
    }

    void tensor_element_count(int64_t* out_count) noexcept
    {
        m_ops->tensor_element_count(get_handle(), out_count);
    }

    void tensor_byte_size(size_t* out_byte_size) noexcept
    {
        m_ops->tensor_byte_size(get_handle(), out_byte_size);
    }

    void tensor_data(void** out_data) noexcept
    {
        m_ops->tensor_data(get_handle(), out_data);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    tensor_bitcast_from(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept
    {
        ice::sonic::Status status;
        m_ops->tensor_bitcast_from(get_handle(), dtype, out_tensor, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    tensor_bitcast_to(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept
    {
        ice::sonic::Status status;
        m_ops->tensor_bitcast_to(get_handle(), dtype, out_tensor, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void tensor_copy(const ice::sonic::TF_TensorOps& dst) noexcept
    {
        m_ops->tensor_copy(get_handle(), dst.get_handle());
    }

    void set_strides(const int64_t* strides, int num_strides) noexcept
    {
        m_ops->set_strides(get_handle(), strides, num_strides);
    }

    void stride(int dim_index, int64_t* out_stride) noexcept
    {
        m_ops->stride(get_handle(), dim_index, out_stride);
    }

    void set_storage_offset(int64_t offset_elements) noexcept
    {
        m_ops->set_storage_offset(get_handle(), offset_elements);
    }

    void storage_offset(int64_t* out_offset_elements) noexcept
    {
        m_ops->storage_offset(get_handle(), out_offset_elements);
    }

    void set_device_index(int device_index) noexcept
    {
        m_ops->set_device_index(get_handle(), device_index);
    }

    void get_device_index(int* out_device_index) noexcept
    {
        m_ops->get_device_index(get_handle(), out_device_index);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> tensor_view(
        const int64_t* dims,
        int num_dims,
        const int64_t* strides,
        int64_t offset_elements,
        TF_Tensor** out_view
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->tensor_view(
            get_handle(),
            dims,
            num_dims,
            strides,
            offset_elements,
            out_view,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
