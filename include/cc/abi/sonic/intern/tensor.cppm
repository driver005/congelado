module;

#include "c/intern/tf_tensor.h"

export module cc_abi_sonic_intern:tensor;

import std;
import :datatype;
import :runtime;
import cc_abi_builder_intern; // For ice::builder::Tensor

export namespace ice::sonic {

class Tensor : public Runtime<Tensor, TF_Tensor, true>
{
public:
    static constexpr std::string_view domain_name = "tensor";

    TF_Tensor_Handle* allocate_tensor(DataTypeEnum dtype, const int64_t* dims, int num_dims, size_t len)
    {
        return m_ops->TF_AllocateTensor(m_host_context, data_type_to_c(dtype), dims, num_dims, len);
    }

    void delete_tensor(TF_Tensor_Handle* tensor)
    {
        m_ops->TF_DeleteTensor(m_host_context, tensor);
    }

    DataTypeEnum get_type(const TF_Tensor_Handle* tensor) const
    {
        return data_type_from_c(m_ops->TF_TensorType(m_host_context, tensor));
    }

    int get_num_dims(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->TF_NumDims(m_host_context, tensor);
    }

    int64_t get_dim(const TF_Tensor_Handle* tensor, int dim_index) const
    {
        return m_ops->TF_Dim(m_host_context, tensor, dim_index);
    }

    int64_t get_num_elements(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->TF_TensorElementCount(m_host_context, tensor);
    }

    size_t get_byte_size(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->TF_TensorByteSize(m_host_context, tensor);
    }

    void* get_data(const TF_Tensor_Handle* tensor) const
    {
        return m_ops->TF_TensorData(m_host_context, tensor);
    }

    void bitcast_from(TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out)
    {
        m_ops->TF_TensorBitcastFrom(m_host_context, src, data_type_to_c(dtype), out);
    }

    void bitcast_to(const TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out)
    {
        m_ops->TF_TensorBitcastTo(m_host_context, src, data_type_to_c(dtype), out);
    }

    void copy(TF_Tensor_Handle* src, TF_Tensor_Handle* dst)
    {
        m_ops->TF_TensorCopy(m_host_context, src, dst);
    }
};

} // namespace ice::sonic
