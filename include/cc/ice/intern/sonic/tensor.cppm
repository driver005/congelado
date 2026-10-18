// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/tensor.h"

export module cc_ice_intern_sonic:tensor;

import std;
import :data_type_enum;
import :registration;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class Tensor : public ice::sonic::Runtime<::TF_TensorOps, ::TF_Tensor>
{
public:
    Tensor(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    Tensor(
        const ice::sonic::Registration& registry,
        const ::TF_Tensor& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit Tensor(const ::TF_TensorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Tensor(const ::TF_TensorOps* ops, const ::TF_Tensor* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_dtype(ice::sonic::DataTypeEnum dtype) const noexcept
    {
        m_ops->set_dtype(get_handle(), dtype.get_value());
    }

    void set_dims(const std::vector<std::int64_t>& dims) const noexcept
    {
        m_ops->set_dims(get_handle(), dims.data(), static_cast<int>(dims.size()));
    }

    void set_byte_size(size_t len) const noexcept
    {
        m_ops->set_byte_size(get_handle(), len);
    }

    void delete_tensor() const noexcept
    {
        m_ops->delete_tensor(get_handle());
    }

    void tensor_type(ice::sonic::DataTypeEnum& out_dtype) const noexcept
    {
        ::TFDataTypeEnum raw_out_dtype{};
        m_ops->tensor_type(get_handle(), &raw_out_dtype);
        out_dtype = ice::sonic::DataTypeEnum{raw_out_dtype};
    }

    void num_dims(int& out_num_dims) const noexcept
    {
        m_ops->num_dims(get_handle(), &out_num_dims);
    }

    void dim(int dim_index, std::int64_t& out_dim) const noexcept
    {
        m_ops->dim(get_handle(), dim_index, &out_dim);
    }

    void tensor_element_count(std::int64_t& out_count) const noexcept
    {
        m_ops->tensor_element_count(get_handle(), &out_count);
    }

    void tensor_byte_size(std::size_t& out_byte_size) const noexcept
    {
        m_ops->tensor_byte_size(get_handle(), &out_byte_size);
    }

    void tensor_data(void** out_data) const noexcept
    {
        m_ops->tensor_data(get_handle(), out_data);
    }

    void tensor_bitcast_from(
        ice::sonic::DataTypeEnum dtype,
        ::TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->tensor_bitcast_from(
            get_handle(),
            dtype.get_value(),
            out_tensor,
            out_status.get_handle()
        );
    }

    void tensor_bitcast_to(
        ice::sonic::DataTypeEnum dtype,
        ::TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->tensor_bitcast_to(
            get_handle(),
            dtype.get_value(),
            out_tensor,
            out_status.get_handle()
        );
    }

    void tensor_copy(const ice::sonic::Tensor& dst) const noexcept
    {
        m_ops->tensor_copy(get_handle(), dst.get_handle());
    }

    void set_strides(const std::vector<std::int64_t>& strides) const noexcept
    {
        m_ops->set_strides(get_handle(), strides.data(), static_cast<int>(strides.size()));
    }

    void stride(int dim_index, std::int64_t& out_stride) const noexcept
    {
        m_ops->stride(get_handle(), dim_index, &out_stride);
    }

    void set_storage_offset(int64_t offset_elements) const noexcept
    {
        m_ops->set_storage_offset(get_handle(), offset_elements);
    }

    void storage_offset(std::int64_t& out_offset_elements) const noexcept
    {
        m_ops->storage_offset(get_handle(), &out_offset_elements);
    }

    void set_device_index(int device_index) const noexcept
    {
        m_ops->set_device_index(get_handle(), device_index);
    }

    void get_device_index(int& out_device_index) const noexcept
    {
        m_ops->get_device_index(get_handle(), &out_device_index);
    }

    void tensor_view(
        const std::vector<std::int64_t>& dims,
        const std::vector<std::int64_t>& strides,
        int64_t offset_elements,
        ::TF_Tensor** out_view,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->tensor_view(
            get_handle(),
            dims.data(),
            static_cast<int>(std::min({dims.size(), strides.size()})),
            strides.data(),
            offset_elements,
            out_view,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
