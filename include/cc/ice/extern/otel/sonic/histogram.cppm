// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/histogram.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/histogram.h"

export module cc_ice_extern_otel_sonic:histogram;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFOtelHistogramOps : public ice::sonic::Runtime<TFOtelHistogramOps, TFOtelHistogramOps>
{
public:
    explicit TFOtelHistogramOps(TFOtelHistogramOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "otel";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> record(double value) noexcept
    {
        ice::sonic::Status status;
        m_ops->record(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
