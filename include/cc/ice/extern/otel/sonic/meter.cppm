// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/meter.h"

export module cc_ice_extern_otel_sonic:meter;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFOtelMeterOps : public ice::sonic::Runtime<TFOtelMeterOps, TFOtelMeterOps>
{
public:
    explicit TFOtelMeterOps(TFOtelMeterOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "otel";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_counter(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::TFOtelCounterOps& out_counter
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->create_counter(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            out_counter.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_histogram(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::TFOtelHistogramOps& out_histogram
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->create_histogram(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            out_histogram.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
