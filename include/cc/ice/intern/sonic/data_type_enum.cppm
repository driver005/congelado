// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/datatype.h"

export module cc_ice_intern_sonic:data_type_enum;

import std;

export namespace ice::sonic {

class DataTypeEnum
{
public:
    enum Value : std::underlying_type_t<::TFDataTypeEnum>
    {
        k_float = ::TF_FLOAT,
        k_double = ::TF_DOUBLE,
        k_int32 = ::TF_INT32,
        k_uint8 = ::TF_UINT8,
        k_int16 = ::TF_INT16,
        k_int8 = ::TF_INT8,
        k_string = ::TF_STRING,
        k_complex64 = ::TF_COMPLEX64,
        k_complex = ::TF_COMPLEX,
        k_int64 = ::TF_INT64,
        k_bool = ::TF_BOOL,
        k_qint8 = ::TF_QINT8,
        k_quint8 = ::TF_QUINT8,
        k_qint32 = ::TF_QINT32,
        k_bfloat16 = ::TF_BFLOAT16,
        k_qint16 = ::TF_QINT16,
        k_quint16 = ::TF_QUINT16,
        k_uint16 = ::TF_UINT16,
        k_complex128 = ::TF_COMPLEX128,
        k_half = ::TF_HALF,
        k_resource = ::TF_RESOURCE,
        k_variant = ::TF_VARIANT,
        k_uint32 = ::TF_UINT32,
        k_uint64 = ::TF_UINT64,
        k_float8_e5m2 = ::TF_FLOAT8_E5M2,
        k_float8_e4m3fn = ::TF_FLOAT8_E4M3FN,
        k_float8_e4m3fnuz = ::TF_FLOAT8_E4M3FNUZ,
        k_float8_e4m3b11fnuz = ::TF_FLOAT8_E4M3B11FNUZ,
        k_float8_e5m2fnuz = ::TF_FLOAT8_E5M2FNUZ,
        k_int4 = ::TF_INT4,
        k_uint4 = ::TF_UINT4,
        k_int2 = ::TF_INT2,
        k_uint2 = ::TF_UINT2,
        k_float4_e2m1fn = ::TF_FLOAT4_E2M1FN
    };

    constexpr DataTypeEnum() noexcept = default;

    constexpr DataTypeEnum(::TFDataTypeEnum value) noexcept :
        m_value{value}
    {
    }

    constexpr DataTypeEnum(Value value) noexcept :
        m_value{static_cast<::TFDataTypeEnum>(value)}
    {
    }

    constexpr ::TFDataTypeEnum get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFDataTypeEnum() const noexcept
    {
        return m_value;
    }

private:
    ::TFDataTypeEnum m_value{};
};

} // namespace ice::sonic
