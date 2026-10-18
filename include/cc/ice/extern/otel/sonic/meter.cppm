// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/meter.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_otel_sonic:meter;

import std;
import :counter;
import :histogram;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class OtelMeter : public ice::sonic::Runtime<::TFOtelMeterOps, ::TFOtelMeter>
{
public:
    OtelMeter(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    OtelMeter(
        const ice::sonic::Registration& registry,
        const ::TFOtelMeter& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit OtelMeter(const ::TFOtelMeterOps* ops) noexcept :
        Runtime(ops)
    {
    }

    OtelMeter(const ::TFOtelMeterOps* ops, const ::TFOtelMeter* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void create_counter(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::OtelCounter& out_counter,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_counter(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            out_counter.get_handle(),
            out_status.get_handle()
        );
    }

    void create_histogram(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::OtelHistogram& out_histogram,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_histogram(
            get_handle(),
            name.get_handle(),
            description.get_handle(),
            unit.get_handle(),
            out_histogram.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
