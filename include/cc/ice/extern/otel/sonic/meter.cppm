// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/meter.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_otel_sonic:meter;

import std;
import :counter;
import :histogram;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFOtelMeterOps : public ice::sonic::Runtime<::TFOtelMeterOps, ::TFOtelMeter>
{
public:
    TFOtelMeterOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFOtelMeterOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFOtelMeter* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFOtelMeterOps(const ::TFOtelMeterOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFOtelMeterOps(const ::TFOtelMeterOps* ops, ::TFOtelMeter* handle) noexcept :
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
        const ice::sonic::TFOtelCounterOps& out_counter,
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
        const ice::sonic::TFOtelHistogramOps& out_histogram,
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
