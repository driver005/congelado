module;

#include "c/intern/tf_datatype.h"

export module cc_abi_value:datatype;

export namespace ice {

enum class DataType
{
    Float = TF_FLOAT,
    Double = TF_DOUBLE,
    Int32 = TF_INT32,
    Uint8 = TF_UINT8,
    Int16 = TF_INT16,
    Int8 = TF_INT8,
    String = TF_STRING,
    Complex64 = TF_COMPLEX64,
    Int64 = TF_INT64,
    Bool = TF_BOOL,
    Qint8 = TF_QINT8,
    Quint8 = TF_QUINT8,
    Qint32 = TF_QINT32,
    Bfloat16 = TF_BFLOAT16,
    Qint16 = TF_QINT16,
    Quint16 = TF_QUINT16,
    Uint16 = TF_UINT16,
    Complex128 = TF_COMPLEX128,
    Half = TF_HALF,
    Resource = TF_RESOURCE,
    Variant = TF_VARIANT,
    Uint32 = TF_UINT32,
    Uint64 = TF_UINT64,
};

inline size_t DataTypeSize(DataType dt)
{
    return TF_DataTypeSize(static_cast<TF_DataType>(dt));
}

} // namespace ice
