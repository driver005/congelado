module;

#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_intern:tensor_factory;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;
import :datatype;
import :tensor;

export namespace aten_xpu {

class SyclTensorFactory
{
public:
    explicit SyclTensorFactory(const SyclOpsTable& ops) noexcept :
        m_ops{ops}
    {
    }

    ::TF_Tensor* empty(
        std::span<const int64_t> dims,
        ice::sonic::DataTypeEnum dtype,
        int device_index,
        ice::sonic::MemorySpace memory_space
    )
    {
        auto* handle = new ::TF_Tensor{};
        handle->plugin_data = (new SyclTensor{m_ops})->get_handle().plugin_data;
        auto& tensor = static_cast<SyclTensor&>(SyclTensor::from_handle(handle));
        tensor.set_device_index(device_index);
        tensor.setMemorySpace(memory_space);
        tensor.set_dtype(dtype);
        tensor.set_dims(dims);
        return handle;
    }

    ::TF_Tensor* empty_strided(
        std::span<const int64_t> dims,
        std::span<const int64_t> strides,
        ice::sonic::DataTypeEnum dtype,
        int device_index,
        ice::sonic::MemorySpace memory_space
    )
    {
        auto* handle = new ::TF_Tensor{};
        handle->plugin_data = (new SyclTensor{m_ops})->get_handle().plugin_data;
        auto& tensor = static_cast<SyclTensor&>(SyclTensor::from_handle(handle));
        tensor.set_device_index(device_index);
        tensor.setMemorySpace(memory_space);
        tensor.set_dtype(dtype);
        tensor.set_dims(dims);
        tensor.set_byte_size(storage_bytes(dims, strides, m_datatype.element_size(dtype)));
        tensor.set_strides(strides);
        return handle;
    }

    std::size_t storage_bytes(
        std::span<const int64_t> dims,
        std::span<const int64_t> strides,
        std::size_t item_size
    ) const noexcept
    {
        int64_t last_element = 0;
        for (std::size_t index = 0; index < dims.size(); ++index) {
            if (dims[index] == 0) {
                return 0;
            }
            last_element += (dims[index] - 1) * strides[index];
        }
        return static_cast<std::size_t>(last_element + 1) * item_size;
    }

private:
    const SyclOpsTable& m_ops;
    SyclDataType m_datatype;
};

} // namespace aten_xpu
