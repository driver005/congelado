// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream_executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/stream_executor.h"

export module cc_ice_extern_stream_executor_sonic:stream_executor;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_StreamExecutorOps : public ice::sonic::Runtime<::TF_StreamExecutorOps, ::TF_StreamExecutor>
{
public:
    TF_StreamExecutorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_StreamExecutorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_StreamExecutor* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_StreamExecutorOps(const ::TF_StreamExecutorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_StreamExecutorOps(const ::TF_StreamExecutorOps* ops, ::TF_StreamExecutor* handle) noexcept :
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
