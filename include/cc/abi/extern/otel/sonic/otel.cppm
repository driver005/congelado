// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/otel/otel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/otel/otel.h"

export module cc_abi_sonic_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Otel : public ice::sonic::Runtime<Otel, TF_Otel>
{
public:
    explicit Otel(TF_Otel* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "otel";

    [[nodiscard]] std::expected<void, ice::Status> create_tracer() noexcept
    {
        ice::Status status;
        m_ops->create_tracer(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tracer_destroy() noexcept
    {
        ice::Status status;
        m_ops->tracer_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> create_meter() noexcept
    {
        ice::Status status;
        m_ops->create_meter(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> meter_destroy() noexcept
    {
        ice::Status status;
        m_ops->meter_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tracer_start_span(const ice::sonic::String& name, int kind) noexcept
    {
        ice::Status status;
        m_ops->tracer_start_span(get_handle(), name.get_handle(), kind, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> span_destroy() noexcept
    {
        ice::Status status;
        m_ops->span_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    span_set_attribute(const ice::sonic::String& key, const ice::sonic::String& value) noexcept
    {
        ice::Status status;
        m_ops->span_set_attribute(
            get_handle(),
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
    span_set_status(int status_code, const ice::sonic::String& description) noexcept
    {
        ice::Status status;
        m_ops->span_set_status(
            get_handle(),
            status_code,
            description.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> span_end() noexcept
    {
        ice::Status status;
        m_ops->span_end(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> meter_create_counter(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit
    ) noexcept
    {
        ice::Status status;
        m_ops->meter_create_counter(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> counter_destroy() noexcept
    {
        ice::Status status;
        m_ops->counter_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> counter_add(double value) noexcept
    {
        ice::Status status;
        m_ops->counter_add(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> meter_create_histogram(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit
    ) noexcept
    {
        ice::Status status;
        m_ops->meter_create_histogram(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> histogram_destroy() noexcept
    {
        ice::Status status;
        m_ops->histogram_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> histogram_record(double value) noexcept
    {
        ice::Status status;
        m_ops->histogram_record(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
