// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/profiler/profiler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/profiler/profiler.h"

export module cc_ice_extern_profiler_sonic:profiler;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ProfilerOps : public ice::sonic::Runtime<::TF_ProfilerOps, ::TF_Profiler>
{
public:
    template<typename Registry>
    TF_ProfilerOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ProfilerOps(
        Registry& registry,
        ::TF_Profiler* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ProfilerOps(const ::TF_ProfilerOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ProfilerOps(const ::TF_ProfilerOps* ops, ::TF_Profiler* handle) noexcept :
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

    void get_device_type(const ice::sonic::String& out_device_type) const noexcept
    {
        m_ops->get_device_type(get_handle(), out_device_type.get_handle());
    }

    void start(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->start(get_handle(), out_status.get_handle());
    }

    void stop(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->stop(get_handle(), out_status.get_handle());
    }

    void collect_data_xspace(
        TF_Tensor** out_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->collect_data_xspace(get_handle(), out_data, out_status.get_handle());
    }
};

} // namespace ice::sonic
