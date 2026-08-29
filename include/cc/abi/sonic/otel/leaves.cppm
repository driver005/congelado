module;

#include "c/extern/otel/otel.h"

export module cc_abi_sonic_otel:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;

export namespace ice::sonic {

// Counter/Histogram/Span — cross-plugin C ABI handle, produced only by the
// parent runtime's factory methods.

class Counter
{
public:
    ~Counter()
    {
        if (m_ops && m_handle) {
            m_ops->counter__destroy(m_handle);
        }
    }

    Counter(const Counter&) = delete;
    Counter& operator=(const Counter&) = delete;
    Counter(Counter&&) = delete;
    Counter& operator=(Counter&&) = delete;

    explicit Counter(TF_Otel* ops, TF_Otel_Counter* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<void, ice::Status> add(double value)
    {
        ice::Status status;
        m_ops->counter__add(m_handle, value, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Otel* m_ops;
    TF_Otel_Counter* m_handle;
};

class Histogram
{
public:
    ~Histogram()
    {
        if (m_ops && m_handle) {
            m_ops->histogram__destroy(m_handle);
        }
    }

    Histogram(const Histogram&) = delete;
    Histogram& operator=(const Histogram&) = delete;
    Histogram(Histogram&&) = delete;
    Histogram& operator=(Histogram&&) = delete;

    explicit Histogram(TF_Otel* ops, TF_Otel_Histogram* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<void, ice::Status> record(double value)
    {
        ice::Status status;
        m_ops->histogram__record(m_handle, value, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Otel* m_ops;
    TF_Otel_Histogram* m_handle;
};

class Span
{
public:
    ~Span()
    {
        if (m_ops && m_handle) {
            m_ops->span__destroy(m_handle);
        }
    }

    Span(const Span&) = delete;
    Span& operator=(const Span&) = delete;
    Span(Span&&) = delete;
    Span& operator=(Span&&) = delete;

    explicit Span(TF_Otel* ops, TF_Otel_Span* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_attribute(const ice::String& key, const ice::String& value)
    {
        ice::Status status;
        m_ops->span__set_attribute(
            m_handle,
            key.get_handle(),
            value.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_status(ice::SpanStatus status_code, const ice::String& description)
    {
        ice::Status status;
        m_ops->span__set_status(
            m_handle,
            ice::span_status_to_c(status_code),
            description.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> end()
    {
        ice::Status status;
        m_ops->span__end(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Otel* m_ops;
    TF_Otel_Span* m_handle;
};

} // namespace ice::sonic
