module;

#include "c/extern/otel/histogram.h"

export module cc_abi_builder_otel:histogram;

import cc_abi_builder_intern;

export namespace ice::builder {

class HistogramBuilder
{
public:
    HistogramBuilder() :
        m_handle{TP_OtelHistogramNew()}
    {
    }

    ~HistogramBuilder()
    {
        TP_OtelHistogramDelete(m_handle);
    }

    HistogramBuilder(const HistogramBuilder&) = delete;
    HistogramBuilder& operator=(const HistogramBuilder&) = delete;

    HistogramBuilder(HistogramBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    HistogramBuilder& operator=(HistogramBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_OtelHistogramDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    HistogramBuilder& set_record(TP_Otel_Histogram_RecordFn callback)
    {

        TP_OtelHistogram_SetRecordCallback(m_handle, callback);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TP_Otel_Histogram* get_handle()
    {
        return m_handle;
    }

    const TP_Otel_Histogram* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Otel_Histogram* m_handle;
};

} // namespace ice::builder
