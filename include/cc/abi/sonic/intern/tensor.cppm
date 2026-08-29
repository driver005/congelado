module;

#include "c/intern/tf_tensor.h"

export module cc_abi_sonic_intern:tensor;

import std;
import cc_abi_primitives;
import :datatype;
import :runtime;

export namespace ice::sonic {

class Tensor : public Runtime<Tensor, TF_TensorOps>
{
public:
    static constexpr std::string_view domain_name = "tensor";

    ice::String get_name() const
    {
        ice::String out;
        m_ops->get_name(m_host_context, out.get_handle());
        return out;
    }

    [[nodiscard]] std::expected<TF_Tensor_Handle*, ice::Status>
    allocate_tensor(DataTypeEnum dtype, const int64_t* dims, int num_dims, size_t len)
    {
        TF_Tensor_Handle* handle =
            m_ops->allocate_tensor(m_host_context, data_type_to_c(dtype), dims, num_dims, len);
        if (handle == nullptr) {
            return std::unexpected{ice::Status{"tensor allocation failed"}};
        }
        return handle;
    }

    void delete_tensor(TF_Tensor_Handle* tensor)
    {
        m_ops->delete_tensor(m_host_context, tensor);
    }

    DataTypeEnum get_type(const TF_Tensor_Handle* tensor) const
    {
        return data_type_from_c(m_ops->tensor_type(m_host_context, tensor));
    }

    int get_num_dims(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->num_dims(m_host_context, tensor);
    }

    int64_t get_dim(const TF_Tensor_Handle* tensor, int dim_index) const
    {
        return m_ops->dim(m_host_context, tensor, dim_index);
    }

    int64_t get_num_elements(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->tensor_element_count(m_host_context, tensor);
    }

    size_t get_byte_size(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->tensor_byte_size(m_host_context, tensor);
    }

    void* get_data(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->tensor_data(m_host_context, tensor);
    }

    [[nodiscard]] std::expected<void, ice::Status>
    bitcast_from(TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out)
    {
        ice::Status status;
        m_ops->tensor_bitcast_from(
            m_host_context,
            src,
            data_type_to_c(dtype),
            out,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    bitcast_to(const TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out)
    {
        ice::Status status;
        m_ops->tensor_bitcast_to(
            m_host_context,
            src,
            data_type_to_c(dtype),
            out,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void copy(TF_Tensor_Handle* src, TF_Tensor_Handle* dst)
    {
        m_ops->tensor_copy(m_host_context, src, dst);
    }
};

} // namespace ice::sonic
