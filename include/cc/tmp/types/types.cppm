module;

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

export module cc_tmp:types_types;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // Native alias to cc_abi ice::DataType
        using DataType = ice::DataType;

        // Standard TensorFlow DataType constants mapped to ice::DataType
        constexpr DataType DT_INVALID = static_cast<DataType>(0);
        constexpr DataType DT_FLOAT = ice::DataType::Float;
        constexpr DataType DT_DOUBLE = ice::DataType::Double;
        constexpr DataType DT_INT32 = ice::DataType::Int32;
        constexpr DataType DT_UINT8 = ice::DataType::Uint8;
        constexpr DataType DT_INT16 = ice::DataType::Int16;
        constexpr DataType DT_INT8 = ice::DataType::Int8;
        constexpr DataType DT_STRING = ice::DataType::String;
        constexpr DataType DT_COMPLEX64 = ice::DataType::Complex64;
        constexpr DataType DT_INT64 = ice::DataType::Int64;
        constexpr DataType DT_BOOL = ice::DataType::Bool;
        constexpr DataType DT_QINT8 = ice::DataType::Qint8;
        constexpr DataType DT_QUINT8 = ice::DataType::Quint8;
        constexpr DataType DT_QINT32 = ice::DataType::Qint32;
        constexpr DataType DT_BFLOAT16 = ice::DataType::Bfloat16;
        constexpr DataType DT_QINT16 = ice::DataType::Qint16;
        constexpr DataType DT_QUINT16 = ice::DataType::Quint16;
        constexpr DataType DT_UINT16 = ice::DataType::Uint16;
        constexpr DataType DT_COMPLEX128 = ice::DataType::Complex128;
        constexpr DataType DT_HALF = ice::DataType::Half;
        constexpr DataType DT_RESOURCE = ice::DataType::Resource;
        constexpr DataType DT_VARIANT = ice::DataType::Variant;
        constexpr DataType DT_UINT32 = ice::DataType::Uint32;
        constexpr DataType DT_UINT64 = ice::DataType::Uint64;

        inline size_t DataTypeSize(DataType dt)
        {
            return ice::DataTypeSize(dt);
        }

        inline std::string_view DataTypeString(DataType dt)
        {
            switch (dt) {
                case DT_FLOAT:
                    return "float";
                case DT_DOUBLE:
                    return "double";
                case DT_INT32:
                    return "int32";
                case DT_UINT8:
                    return "uint8";
                case DT_INT16:
                    return "int16";
                case DT_INT8:
                    return "int8";
                case DT_STRING:
                    return "string";
                case DT_COMPLEX64:
                    return "complex64";
                case DT_INT64:
                    return "int64";
                case DT_BOOL:
                    return "bool";
                case DT_QINT8:
                    return "qint8";
                case DT_QUINT8:
                    return "quint8";
                case DT_QINT32:
                    return "qint32";
                case DT_BFLOAT16:
                    return "bfloat16";
                case DT_QINT16:
                    return "qint16";
                case DT_QUINT16:
                    return "quint16";
                case DT_UINT16:
                    return "uint16";
                case DT_COMPLEX128:
                    return "complex128";
                case DT_HALF:
                    return "half";
                case DT_RESOURCE:
                    return "resource";
                case DT_VARIANT:
                    return "variant";
                case DT_UINT32:
                    return "uint32";
                case DT_UINT64:
                    return "uint64";
                default:
                    return "invalid";
            }
        }

        constexpr inline ice::DataType DataTypeToIce(DataType dt)
        {
            return dt;
        }

        constexpr inline DataType DataTypeFromIce(ice::DataType ice_dt)
        {
            return ice_dt;
        }

    } // namespace tensorflow

} // export
