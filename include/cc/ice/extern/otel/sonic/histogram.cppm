// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/histogram.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/histogram.h"

export module cc_ice_extern_otel_sonic:histogram;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFOtelHistogramOps : public ice::sonic::Runtime<::TFOtelHistogramOps, ::TFOtelHistogram>
{
public:
    template<typename Registry>
    TFOtelHistogramOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFOtelHistogramOps(
        Registry& registry,
        ::TFOtelHistogram* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFOtelHistogramOps(const ::TFOtelHistogramOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFOtelHistogramOps(const ::TFOtelHistogramOps* ops, ::TFOtelHistogram* handle) noexcept :
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

    void record(double value, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->record(get_handle(), value, out_status.get_handle());
    }
};

} // namespace ice::sonic
