// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/event.h"

export module cc_ice_extern_stream_executor_sonic:event;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_EventOps : public ice::sonic::Runtime<TF_EventOps, TF_EventOps>
{
public:
    explicit TF_EventOps(TF_EventOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "stream_executor";

    [[nodiscard]] std::expected<void, ice::Status>
    elapsed_time(const ice::sonic::TF_EventOps& end, float* out_milliseconds) noexcept
    {
        ice::Status status;
        m_ops->elapsed_time(get_handle(), end.get_handle(), out_milliseconds status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    export_ipc(TF_IpcEventHandle* out_handle) noexcept
    {
        ice::Status status;
        m_ops->export_ipc(get_handle(), out_handle status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_native_handle(void** out_handle) noexcept
    {
        ice::Status status;
        m_ops->get_native_handle(get_handle(), out_handle status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
