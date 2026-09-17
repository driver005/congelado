// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/durable.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/durable.h"

export module cc_abi_sonic_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_DurableOps : public ice::sonic::Runtime<TF_DurableOps, TF_DurableOps>
{
public:
    explicit TF_DurableOps(TF_DurableOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "jobber";

    [[nodiscard]] std::expected<void, ice::Status> signal(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_StringOps& signal_name,
        const ice::sonic::TF_StringOps& payload
    ) noexcept
    {
        ice::Status status;
        m_ops->signal(
            get_handle(),
            job.get_handle(),
            signal_name.get_handle(),
            payload.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> checkpoint(
        const ice::sonic::TF_JobOps& job,
        TFDurableAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops
            ->checkpoint(get_handle(), job.get_handle(), completion, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    restore_checkpoint(const ice::sonic::TF_JobOps& job) noexcept
    {
        ice::Status status;
        m_ops->restore_checkpoint(get_handle(), job.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
