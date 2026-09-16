// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/timepoint/timepoint.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/timepoint/timepoint.h"

export module cc_abi_sonic_timepoint;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Timepoint : public ice::sonic::Runtime<Timepoint, TF_TimePoint>
{
public:
    explicit Timepoint(TF_TimePoint* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "timepoint";

    [[nodiscard]] std::expected<void, ice::Status>
    new_time_point(int64_t ticks, int64_t ratio_num, int64_t ratio_den) noexcept
    {
        ice::Status status;
        m_ops->new_time_point(get_handle(), ticks, ratio_num, ratio_den, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_duration_since_epoch() noexcept
    {
        ice::Status status;
        m_ops->get_duration_since_epoch(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
