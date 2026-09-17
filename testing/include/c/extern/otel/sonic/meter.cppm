// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/meter.h"

export module cc_abi_sonic_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
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

    [[nodiscard]] std::expected<void, ice::Status> create_counter(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& description,
        const ice::sonic::TF_StringOps& unit,
        const ice::sonic::TFOtelCounterOps& out_counter
    ) noexcept
    {
        ice::Status status;
        m_ops->create_counter(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            out_counter.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> create_histogram(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& description,
        const ice::sonic::TF_StringOps& unit,
        const ice::sonic::TFOtelHistogramOps& out_histogram
    ) noexcept
    {
        ice::Status status;
        m_ops->create_histogram(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            out_histogram.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
