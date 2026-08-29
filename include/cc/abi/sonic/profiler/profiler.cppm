module;

#include "c/extern/profiler/profiler.h"

export module cc_abi_sonic_profiler;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing profiler handle. Unlike ice::sonic::Cache and the
// other domains under cc/abi/sonic, this always crosses the TF_Profiler_* C ABI — no
// in-process RegistrationRuntime fast path. A profiler plugin is exactly the kind of
// independently-built, possibly-different-toolchain third party this codebase needs real
// binary compatibility with; handing a raw C++ object across that boundary and calling virtual
// methods on it directly (the in-process shortcut every other domain used to have) isn't safe
// for that case, so this domain never grew one.
class Profiler : public ice::sonic::Runtime<Profiler, TF_Profiler>
{
public:
    explicit Profiler(TF_Profiler* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "profiler";

    ice::String get_device_type() const noexcept
    {
        ice::String tf_device_type;
        m_ops->get_device_type(get_handle(), tf_device_type.get_handle());
        return tf_device_type;
    }

    [[nodiscard]] std::expected<void, ice::Status> start() noexcept
    {
        ice::Status status;
        m_ops->start(get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept
    {
        ice::Status status;
        m_ops->stop(get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    collect_data_xspace(std::uint8_t* buffer, std::size_t* size_in_bytes) noexcept
    {
        ice::Status status;
        m_ops->collect_data_xspace(get_handle(), buffer, size_in_bytes, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
