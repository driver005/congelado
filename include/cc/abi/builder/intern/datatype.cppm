module;

#include "c/intern/tf_datatype.h"

export module cc_abi_builder_intern:datatype;

import std;

export namespace ice::builder {

enum class DataTypeEnum
{
    Float            = TF_FLOAT,
    Double           = TF_DOUBLE,
    Int32            = TF_INT32,
    Uint8            = TF_UINT8,
    Int16            = TF_INT16,
    Int8             = TF_INT8,
    String           = TF_STRING,
    Complex64        = TF_COMPLEX64,
    Complex          = TF_COMPLEX,
    Int64            = TF_INT64,
    Bool             = TF_BOOL,
    Qint8            = TF_QINT8,
    Quint8           = TF_QUINT8,
    Qint32           = TF_QINT32,
    Bfloat16         = TF_BFLOAT16,
    Qint16           = TF_QINT16,
    Quint16          = TF_QUINT16,
    Uint16           = TF_UINT16,
    Complex128       = TF_COMPLEX128,
    Half             = TF_HALF,
    Resource         = TF_RESOURCE,
    Variant          = TF_VARIANT,
    Uint32           = TF_UINT32,
    Uint64           = TF_UINT64,
    Float8E5M2       = TF_FLOAT8_E5M2,
    Float8E4M3FN     = TF_FLOAT8_E4M3FN,
    Float8E4M3FNUZ   = TF_FLOAT8_E4M3FNUZ,
    Float8E4M3B11FNUZ = TF_FLOAT8_E4M3B11FNUZ,
    Float8E5M2FNUZ   = TF_FLOAT8_E5M2FNUZ,
    Int4             = TF_INT4,
    Uint4            = TF_UINT4,
    Int2             = TF_INT2,
    Uint2            = TF_UINT2,
    Float4E2M1FN     = TF_FLOAT4_E2M1FN
};

// Converters between the C++ enum class and the C ABI enum — centralises the
// static_cast so call sites don't need to know the underlying type.
inline TF_DataType_Enum data_type_to_c(DataTypeEnum dt) noexcept {
    return static_cast<TF_DataType_Enum>(dt);
}
inline DataTypeEnum data_type_from_c(TF_DataType_Enum dt) noexcept {
    return static_cast<DataTypeEnum>(dt);
}

class DataType
{
public:
    virtual ~DataType() = default;

    virtual size_t data_type_size(DataTypeEnum dt) = 0;

    static TF_DataType* get_generic_vtable() {
        static TF_DataType vtable = {
            .struct_size = sizeof(TF_DataType),
            .TF_DataTypeSize = [](void* ctx, TF_DataType_Enum dt) -> size_t {
                return ctx_as<DataType>(ctx)->data_type_size(data_type_from_c(dt));
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
