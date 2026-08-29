module;

#include "c/extern/otel/otel.h"

export module cc_abi_sonic_otel;

import std;
export import :leaves;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Tracer
{
public:
    ~Tracer()
    {
        if (m_ops && m_handle) {
            m_ops->tracer__destroy(m_handle);
        }
    }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
    Tracer(Tracer&&) = delete;
    Tracer& operator=(Tracer&&) = delete;

    explicit Tracer(TF_Otel* ops, void* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    std::unique_ptr<ice::sonic::Span> start_span(const ice::String& name, ice::SpanKind kind) noexcept
    {
        ice::Status status;
        void* handle = m_ops->tracer__start_span(
            m_handle,
            name.get_handle(),
            ice::span_kind_to_c(kind),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->span__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::Span>(m_ops, handle);
    }

private:
    TF_Otel* m_ops;
    void* m_handle;
};

class Meter
{
public:
    ~Meter()
    {
        if (m_ops && m_handle) {
            m_ops->meter__destroy(m_handle);
        }
    }

    Meter(const Meter&) = delete;
    Meter& operator=(const Meter&) = delete;
    Meter(Meter&&) = delete;
    Meter& operator=(Meter&&) = delete;

    explicit Meter(TF_Otel* ops, void* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    std::unique_ptr<ice::sonic::Counter>
    create_counter(const ice::String& name, const ice::String& description, const ice::String& unit) noexcept
    {
        ice::Status status;
        void* handle = m_ops->meter__create_counter(
            m_handle,
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->counter__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::Counter>(m_ops, handle);
    }

    std::unique_ptr<ice::sonic::Histogram> create_histogram(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    ) noexcept
    {
        ice::Status status;
        void* handle = m_ops->meter__create_histogram(
            m_handle,
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->histogram__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::Histogram>(m_ops, handle);
    }

private:
    TF_Otel* m_ops;
    void* m_handle;
};

// Runtime — the mainframe-facing otel handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Otel : public ice::sonic::Runtime<Otel, TF_Otel>
{
public:
    explicit Otel(TF_Otel* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "otel";

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::Tracer>, ice::Status> get_tracer() noexcept
    {
        ice::Status status;
        void* handle = m_ops->get_tracer(get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->tracer__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::Tracer>(m_ops, handle);
    }

    std::unique_ptr<ice::sonic::Meter> get_meter() noexcept
    {
        ice::Status status;
        void* handle = m_ops->get_meter(get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->meter__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::Meter>(m_ops, handle);
    }
};

} // namespace ice::sonic
