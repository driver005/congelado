// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/profiler/profiler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/profiler/profiler.h"

export module cc_ice_extern_profiler_sonic:profiler;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ProfilerOps : public ice::sonic::Runtime<TF_ProfilerOps, TF_ProfilerOps>
{
public:
    explicit TF_ProfilerOps(TF_ProfilerOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "profiler";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void get_device_type(const ice::sonic::String& out_device_type) noexcept
    {
        m_ops->get_device_type(get_handle(), out_device_type.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> start() noexcept
    {
        ice::sonic::Status status;
        m_ops->start(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> stop() noexcept
    {
        ice::sonic::Status status;
        m_ops->stop(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    collect_data_xspace(TF_Tensor** out_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->collect_data_xspace(get_handle(), out_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
