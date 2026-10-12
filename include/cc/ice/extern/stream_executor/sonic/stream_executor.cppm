// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream_executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/stream_executor.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_stream_executor_sonic:stream_executor;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class StreamExecutor : public ice::sonic::Runtime<::TF_StreamExecutorOps, ::TF_StreamExecutor>
{
public:
    StreamExecutor(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    StreamExecutor(
        const ice::sonic::Registration& registry,
        ::TF_StreamExecutor* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit StreamExecutor(const ::TF_StreamExecutorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    StreamExecutor(const ::TF_StreamExecutorOps* ops, ::TF_StreamExecutor* handle) noexcept :
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
