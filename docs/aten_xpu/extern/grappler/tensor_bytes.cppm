module;

#include "cc/proto/tensor_properties.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:tensor_bytes;

import std;

export namespace aten_xpu {

// The size in bytes of a tensor the host's shape inference described: element size times the
// product of the dimensions. Nothing when the rank or any dimension is unknown, or the dtype has no
// fixed size.
class SyclTensorBytes
{
public:
    std::optional<std::uint64_t> of(const tensorflow::TensorProperties& properties) const
    {
        const auto element = elementSize(properties.dtype());
        if (!element || properties.shape().unknown_rank()) {
            return std::nullopt;
        }
        std::uint64_t count = 1;
        for (const auto& dimension: properties.shape().dim()) {
            if (dimension.size() < 0) {
                return std::nullopt;
            }
            count *= static_cast<std::uint64_t>(dimension.size());
        }
        return count * *element;
    }

    std::optional<std::uint64_t> elementSize(tensorflow::DataType type) const
    {
        switch (type) {
            case tensorflow::DT_BOOL:
            case tensorflow::DT_INT8:
            case tensorflow::DT_UINT8:
            case tensorflow::DT_QINT8:
            case tensorflow::DT_QUINT8:
                return 1;
            case tensorflow::DT_HALF:
            case tensorflow::DT_BFLOAT16:
            case tensorflow::DT_INT16:
            case tensorflow::DT_UINT16:
            case tensorflow::DT_QINT16:
            case tensorflow::DT_QUINT16:
                return 2;
            case tensorflow::DT_FLOAT:
            case tensorflow::DT_INT32:
            case tensorflow::DT_UINT32:
            case tensorflow::DT_QINT32:
                return 4;
            case tensorflow::DT_DOUBLE:
            case tensorflow::DT_INT64:
            case tensorflow::DT_UINT64:
            case tensorflow::DT_COMPLEX64:
                return 8;
            case tensorflow::DT_COMPLEX128:
                return 16;
            default:
                return std::nullopt;
        }
    }
};

} // namespace aten_xpu
