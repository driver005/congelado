// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/span.h"

export module cc_ice_extern_otel_sonic:span;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFOtelSpanOps : public ice::sonic::Runtime<TFOtelSpanOps, TFOtelSpanOps>
{
public:
    explicit TFOtelSpanOps(TFOtelSpanOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "otel";

    [[nodiscard]] std::expected<void, ice::Status>
    set_attribute(const ice::sonic::String& key, const ice::sonic::String& value) noexcept
    {
        ice::Status status;
        m_ops->set_attribute(
            get_handle(),
            key.get_handle(),
            value.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_status(int status_code, const ice::sonic::String& description) noexcept
    {
        ice::Status status;
        m_ops->set_status(get_handle(), status_code, description.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> end() noexcept
    {
        ice::Status status;
        m_ops->end(get_handle(), status.get_handle());

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
