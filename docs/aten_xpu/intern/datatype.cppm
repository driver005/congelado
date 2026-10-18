module;

#include "include/c/intern/tensor.h"

export module aten_xpu_intern:datatype;

import std;
import cc_ice_intern_sonic;

export namespace aten_xpu {

class SyclDataType
{
public:
    std::size_t element_size(ice::sonic::DataTypeEnum dtype) const noexcept
    {
        switch (dtype) {
            case ice::sonic::DataTypeEnum::k_bool:
            case ice::sonic::DataTypeEnum::k_uint8:
            case ice::sonic::DataTypeEnum::k_int8:
            case ice::sonic::DataTypeEnum::k_qint8:
            case ice::sonic::DataTypeEnum::k_quint8:
            case ice::sonic::DataTypeEnum::k_float8_e5m2:
            case ice::sonic::DataTypeEnum::k_float8_e4m3fn:
            case ice::sonic::DataTypeEnum::k_float8_e4m3fnuz:
            case ice::sonic::DataTypeEnum::k_float8_e4m3b11fnuz:
            case ice::sonic::DataTypeEnum::k_float8_e5m2fnuz:
                return 1;
            case ice::sonic::DataTypeEnum::k_half:
            case ice::sonic::DataTypeEnum::k_bfloat16:
            case ice::sonic::DataTypeEnum::k_int16:
            case ice::sonic::DataTypeEnum::k_uint16:
            case ice::sonic::DataTypeEnum::k_qint16:
            case ice::sonic::DataTypeEnum::k_quint16:
                return 2;
            case ice::sonic::DataTypeEnum::k_float:
            case ice::sonic::DataTypeEnum::k_int32:
            case ice::sonic::DataTypeEnum::k_uint32:
            case ice::sonic::DataTypeEnum::k_qint32:
                return 4;
            case ice::sonic::DataTypeEnum::k_double:
            case ice::sonic::DataTypeEnum::k_int64:
            case ice::sonic::DataTypeEnum::k_uint64:
            case ice::sonic::DataTypeEnum::k_complex64:
                return 8;
            case ice::sonic::DataTypeEnum::k_complex128:
                return 16;
            default:
                return 1;
        }
    }
};

} // namespace aten_xpu
