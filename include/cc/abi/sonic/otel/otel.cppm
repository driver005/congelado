module;

#include "c/extern/otel/otel.h"

export module cc_abi_sonic_otel;

export import :leaves;
export import :enums;
import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
import :leaves;


export namespace ice::sonic {

class TracerRuntime : public ice::builder::Tracer
{
public:
    ~TracerOtel() { if (m_handle && m_ops) m_ops->tracer__destroy(m_handle); }

    TracerOtel(const TracerRuntime&) = delete;
    TracerRuntime& operator=(const TracerRuntime&) = delete;
    TracerOtel(TracerRuntime&&) = delete;
    TracerRuntime& operator=(TracerRuntime&&) = delete;

    explicit TracerOtel(TF_Otel* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<std::unique_ptr<ice::builder::Span>, ice::Status>
    start_span(const ice::String& name, int kind)
    {


        ice::Status status;
        void* handle =
            m_ops->tracer__start_span(m_handle, name.get_handle(), kind, status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->span__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<SpanRuntime>(m_ops, handle);
    }

private:
    TF_Otel* m_ops; void* m_handle;
};

class MeterRuntime : public ice::builder::Meter
{
public:
    ~MeterOtel() { if (m_handle && m_ops) m_ops->meter__destroy(m_handle); }

    MeterOtel(const MeterRuntime&) = delete;
    MeterRuntime& operator=(const MeterRuntime&) = delete;
    MeterOtel(MeterRuntime&&) = delete;
    MeterRuntime& operator=(MeterRuntime&&) = delete;

    explicit MeterOtel(TF_Otel* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<std::unique_ptr<ice::builder::Counter>, ice::Status>
    create_counter(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    )
    {


        ice::Status status;
        void* handle = m_ops->meter__create_counter(m_handle, name.get_handle(), description.get_handle(), unit.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                this->m_ops->counter__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<CounterRuntime>(m_ops, handle);
    }

    std::expected<std::unique_ptr<ice::builder::Histogram>, ice::Status>
    create_histogram(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    )
    {


        ice::Status status;
        void* handle = m_ops->meter__create_histogram(m_handle, name.get_handle(), description.get_handle(), unit.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->histogram__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<HistogramRuntime>(m_ops, handle);
    }

private:
    TF_Otel* m_ops; void* m_handle;
};

// Runtime — the mainframe-facing otel handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Otel : public ice::sonic::Runtime<Otel, TF_Otel, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "otel";

    std::expected<std::unique_ptr<ice::builder::Tracer>, ice::Status>
    get_tracer()
    {


        ice::Status status;
        void* handle = this->m_ops->get_tracer(this->get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->tracer__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<TracerRuntime>(this->m_ops, handle);
    }

    std::expected<std::unique_ptr<ice::builder::Meter>, ice::Status>
    get_meter()
    {


        ice::Status status;
        void* handle = this->m_ops->get_meter(this->get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->meter__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<MeterRuntime>(this->m_ops, handle);
    }

public:
    explicit Otel(TF_Otel* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
