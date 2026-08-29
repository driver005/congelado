module;

#include "c/extern/otel/otel.h"

export module cc_abi_sonic_otel:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
export namespace ice::sonic::otel {

// CounterRuntime/HistogramRuntime/SpanRuntime — cross-plugin C ABI handle, produced only by the parent runtime's factory methods.

class CounterRuntime : public ice::builder::Counter
{
public:
    ~CounterRuntime() { if (m_handle && m_ops) m_ops->counter__destroy(m_handle); }

    CounterRuntime(const CounterRuntime&) = delete;
    CounterRuntime& operator=(const CounterRuntime&) = delete;
    CounterRuntime(CounterRuntime&&) = delete;
    CounterRuntime& operator=(CounterRuntime&&) = delete;

    explicit CounterRuntime(TF_Otel* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<void, ice::Status> add(double value)
    {


        ice::Status status;
        m_ops->counter__add(m_handle, value, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Otel* m_ops; void* m_handle;
};

class HistogramRuntime : public ice::builder::Histogram
{
public:
    ~HistogramRuntime() { if (m_handle && m_ops) m_ops->histogram__destroy(m_handle); }

    HistogramRuntime(const HistogramRuntime&) = delete;
    HistogramRuntime& operator=(const HistogramRuntime&) = delete;
    HistogramRuntime(HistogramRuntime&&) = delete;
    HistogramRuntime& operator=(HistogramRuntime&&) = delete;

    explicit HistogramRuntime(TF_Otel* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<void, ice::Status> record(double value)
    {


        ice::Status status;
        m_ops->histogram__record(m_handle, value, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Otel* m_ops; void* m_handle;
};

class SpanRuntime : public ice::builder::Span
{
public:
    ~SpanRuntime() { if (m_handle && m_ops) m_ops->span__destroy(m_handle); }

    SpanRuntime(const SpanRuntime&) = delete;
    SpanRuntime& operator=(const SpanRuntime&) = delete;
    SpanRuntime(SpanRuntime&&) = delete;
    SpanRuntime& operator=(SpanRuntime&&) = delete;

    explicit SpanRuntime(TF_Otel* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<void, ice::Status> set_attribute(
        const ice::String& key, const ice::String& value
    )
    {


        ice::Status status;
        m_ops->span__set_attribute(m_handle, key.get_handle(), value.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    set_status(int status_code, const ice::String& description)
    {


        ice::Status status;
        m_ops->span__set_status(m_handle, status_code, description.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> end()
    {


        ice::Status status;
        m_ops->span__end(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Otel* m_ops; void* m_handle;
};

} // namespace ice::sonic
