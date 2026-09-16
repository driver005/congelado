// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/duration/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/duration/duration.h"

export module cc_abi_sonic_duration;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Duration : public ice::sonic::Runtime<Duration, TF_Duration>
{
public:
    explicit Duration(TF_Duration* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "duration";

    [[nodiscard]] std::expected<void, ice::Status>
    new_duration(int64_t ticks, int64_t ratio_num, int64_t ratio_den) noexcept
    {
        ice::Status status;
        m_ops->new_duration(get_handle(), ticks, ratio_num, ratio_den, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_ticks() noexcept
    {
        ice::Status status;
        m_ops->get_ticks(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_ratio_num() noexcept
    {
        ice::Status status;
        m_ops->get_ratio_num(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_ratio_den() noexcept
    {
        ice::Status status;
        m_ops->get_ratio_den(get_handle(), status.get_handle());

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
