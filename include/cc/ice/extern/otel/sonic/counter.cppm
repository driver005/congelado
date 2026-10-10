// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/counter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/counter.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_otel_sonic:counter;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFOtelCounterOps : public ice::sonic::Runtime<::TFOtelCounterOps, ::TFOtelCounter>
{
public:
    TFOtelCounterOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFOtelCounterOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFOtelCounter* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFOtelCounterOps(const ::TFOtelCounterOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFOtelCounterOps(const ::TFOtelCounterOps* ops, ::TFOtelCounter* handle) noexcept :
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

    void add(double value, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->add(get_handle(), value, out_status.get_handle());
    }
};

} // namespace ice::sonic
