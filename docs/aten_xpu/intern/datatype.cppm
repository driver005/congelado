module;

#include "include/c/intern/tensor.h"

export module aten_xpu_intern:datatype;

import std;

export namespace aten_xpu {

class SyclDataType
{
public:
    std::size_t element_size(TFDataTypeEnum dtype) const noexcept
    {
        switch (dtype) {
            case TF_BOOL:
            case TF_UINT8:
            case TF_INT8:
            case TF_QINT8:
            case TF_QUINT8:
            case TF_FLOAT8_E5M2:
            case TF_FLOAT8_E4M3FN:
            case TF_FLOAT8_E4M3FNUZ:
            case TF_FLOAT8_E4M3B11FNUZ:
            case TF_FLOAT8_E5M2FNUZ:
                return 1;
            case TF_HALF:
            case TF_BFLOAT16:
            case TF_INT16:
            case TF_UINT16:
            case TF_QINT16:
            case TF_QUINT16:
                return 2;
            case TF_FLOAT:
            case TF_INT32:
            case TF_UINT32:
            case TF_QINT32:
                return 4;
            case TF_DOUBLE:
            case TF_INT64:
            case TF_UINT64:
            case TF_COMPLEX64:
                return 8;
            case TF_COMPLEX128:
                return 16;
            default:
                return 1;
        }
    }
};

} // namespace aten_xpu
