module;

#include "c/extern/io/enums.h"
#include "c/extern/otel/enums.h"
#include "c/extern/payload/payload.h"
#include "c/intern/tf_datatype.h"

export module cc_abi_primitives:types;

import std;
import :string;

export namespace ice {

// DataTypeEnum — C++ spelling of the TF_DataType_Enum C ABI enum.
enum class DataTypeEnum
{
    Float = TF_FLOAT,
    Double = TF_DOUBLE,
    Int32 = TF_INT32,
    Uint8 = TF_UINT8,
    Int16 = TF_INT16,
    Int8 = TF_INT8,
    String = TF_STRING,
    Complex64 = TF_COMPLEX64,
    Complex = TF_COMPLEX,
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
    Float8E5M2 = TF_FLOAT8_E5M2,
    Float8E4M3FN = TF_FLOAT8_E4M3FN,
    Float8E4M3FNUZ = TF_FLOAT8_E4M3FNUZ,
    Float8E4M3B11FNUZ = TF_FLOAT8_E4M3B11FNUZ,
    Float8E5M2FNUZ = TF_FLOAT8_E5M2FNUZ,
    Int4 = TF_INT4,
    Uint4 = TF_UINT4,
    Int2 = TF_INT2,
    Uint2 = TF_UINT2,
    Float4E2M1FN = TF_FLOAT4_E2M1FN
};

// Converters between the C++ enum class and the C ABI enum — centralise the
// static_cast so call sites don't need to know the underlying type.
inline TF_DataType_Enum datatype_to_c(DataTypeEnum dt) noexcept
{
    return static_cast<TF_DataType_Enum>(dt);
}

inline DataTypeEnum datatype_from_c(TF_DataType_Enum dt) noexcept
{
    return static_cast<DataTypeEnum>(dt);
}

// Method — C++ spelling of the TF_IO_Method C ABI enum.
enum class Method
{
    Get = TF_IO_GET,
    Post = TF_IO_POST,
    Put = TF_IO_PUT,
    Delete = TF_IO_DELETE,
    Patch = TF_IO_PATCH,
    Head = TF_IO_HEAD,
    Options = TF_IO_OPTIONS,
    Connect = TF_IO_CONNECT,
    Trace = TF_IO_TRACE,
};

inline TF_IO_Method method_to_c(Method m) noexcept
{
    return static_cast<TF_IO_Method>(m);
}

inline Method method_from_c(TF_IO_Method m) noexcept
{
    return static_cast<Method>(m);
}

// PayloadType — payload kind (workflow/task x input/output).
enum class PayloadType
{
    WorkflowInput,
    WorkflowOutput,
    TaskInput,
    TaskOutput,
};

inline TF_Payload_Type payload_type_to_c(PayloadType type) noexcept
{
    switch (type) {
        case PayloadType::WorkflowInput:
            return TF_PAYLOAD_WORKFLOW_INPUT;
        case PayloadType::WorkflowOutput:
            return TF_PAYLOAD_WORKFLOW_OUTPUT;
        case PayloadType::TaskInput:
            return TF_PAYLOAD_TASK_INPUT;
        case PayloadType::TaskOutput:
            return TF_PAYLOAD_TASK_OUTPUT;
    }
}

inline PayloadType payload_type_from_c(TF_Payload_Type type) noexcept
{
    switch (type) {
        case TF_PAYLOAD_WORKFLOW_INPUT:
            return PayloadType::WorkflowInput;
        case TF_PAYLOAD_WORKFLOW_OUTPUT:
            return PayloadType::WorkflowOutput;
        case TF_PAYLOAD_TASK_INPUT:
            return PayloadType::TaskInput;
        case TF_PAYLOAD_TASK_OUTPUT:
            return PayloadType::TaskOutput;
    }
}

// SpanKind / SpanStatus — C++ spellings of the TF_Otel_SpanKind / TF_Otel_SpanStatus C ABI enums.
enum class SpanKind
{
    Internal = TF_OTEL_INTERNAL,
    Server = TF_OTEL_SERVER,
    Client = TF_OTEL_CLIENT,
    Producer = TF_OTEL_PRODUCER,
    Consumer = TF_OTEL_CONSUMER,
};

enum class SpanStatus
{
    Unset = TF_OTEL_UNSET,
    Ok = TF_OTEL_OK,
    Error = TF_OTEL_ERROR,
};

inline TF_Otel_SpanKind span_kind_to_c(SpanKind kind) noexcept
{
    return static_cast<TF_Otel_SpanKind>(kind);
}

inline SpanKind span_kind_from_c(TF_Otel_SpanKind kind) noexcept
{
    return static_cast<SpanKind>(kind);
}

inline TF_Otel_SpanStatus span_status_to_c(SpanStatus status) noexcept
{
    return static_cast<TF_Otel_SpanStatus>(status);
}

inline SpanStatus span_status_from_c(TF_Otel_SpanStatus status) noexcept
{
    return static_cast<SpanStatus>(status);
}

// SearchQuery — owning value type the mainframe builds up before calling
// Search::search(). Pure C++, zero C-ABI/TF_* knowledge (the sonic adapter converts this into
// the raw TF_Search_Query C struct only when crossing into a cross-plugin backend).
class SearchQuery
{
public:
    SearchQuery() noexcept = default;

    SearchQuery& set_query(const ice::String& value) noexcept
    {
        m_query = value;
        return *this;
    }

    SearchQuery& set_free_text(const ice::String& value) noexcept
    {
        m_free_text = value;
        return *this;
    }

    SearchQuery& set_start(std::int64_t start) noexcept
    {
        m_start = start;
        return *this;
    }

    SearchQuery& set_size(std::int64_t size) noexcept
    {
        m_size = size;
        return *this;
    }

    SearchQuery& set_sort(const ice::String& value) noexcept
    {
        m_sort = value;
        return *this;
    }

    const ice::String& get_query() const noexcept
    {
        return m_query;
    }

    const ice::String& get_free_text() const noexcept
    {
        return m_free_text;
    }

    std::int64_t get_start() const noexcept
    {
        return m_start;
    }

    std::int64_t get_size() const noexcept
    {
        return m_size;
    }

    const ice::String& get_sort() const noexcept
    {
        return m_sort;
    }

private:
    ice::String m_query;
    ice::String m_free_text;
    std::int64_t m_start{0};
    std::int64_t m_size{10};
    ice::String m_sort;
};

} // namespace ice
