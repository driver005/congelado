module;

#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_intern:tensor_factory;

import std;
import :handle;
import :tensor;

export namespace aten_xpu {

class SyclTensorFactory
{
public:
    SyclTensorFactory() = delete;

    static ::TF_Tensor* empty(
        std::span<const int64_t> dims,
        TFDataTypeEnum dtype,
        int device_index,
        TF_MemorySpace memory_space
    )
    {
        auto* handle = SyclTensor::create_handle();
        auto& tensor = SyclHandle::resolve_raw<SyclTensor>(handle);
        tensor.set_device_index(device_index);
        tensor.setMemorySpace(memory_space);
        tensor.set_dtype(dtype);
        tensor.set_dims(dims.data(), static_cast<int>(dims.size()));
        return handle;
    }

    static ::TF_Tensor* empty_strided(
        std::span<const int64_t> dims,
        std::span<const int64_t> strides,
        TFDataTypeEnum dtype,
        int device_index,
        TF_MemorySpace memory_space
    )
    {
        auto* handle = SyclTensor::create_handle();
        auto& tensor = SyclHandle::resolve_raw<SyclTensor>(handle);
        tensor.set_device_index(device_index);
        tensor.setMemorySpace(memory_space);
        tensor.set_dtype(dtype);
        tensor.set_dims(dims.data(), static_cast<int>(dims.size()));
        tensor.set_byte_size(storage_bytes(dims, strides, SyclTensor::element_size(dtype)));
        tensor.set_strides(strides.data(), static_cast<int>(strides.size()));
        return handle;
    }

    static std::size_t storage_bytes(
        std::span<const int64_t> dims,
        std::span<const int64_t> strides,
        std::size_t item_size
    ) noexcept
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
};

} // namespace aten_xpu
