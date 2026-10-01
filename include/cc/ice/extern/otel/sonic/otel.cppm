// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/otel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/otel.h"

export module cc_ice_extern_otel_sonic:otel;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_OtelOps : public ice::sonic::Runtime<::TF_OtelOps, ::TF_Otel>
{
public:
    template<typename Registry>
    TF_OtelOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_OtelOps(
        Registry& registry,
        ::TF_Otel* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_OtelOps(const ::TF_OtelOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_OtelOps(const ::TF_OtelOps* ops, ::TF_Otel* handle) noexcept :
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
};

} // namespace ice::sonic
