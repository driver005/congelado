// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/observe.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/observe.h"

export module cc_abi_sonic_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ObserveOps : public ice::sonic::Runtime<TF_ObserveOps, TF_ObserveOps>
{
public:
    explicit TF_ObserveOps(TF_ObserveOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "jobber";

    [[nodiscard]] std::expected<void, ice::Status> get_status(
        const ice::sonic::TF_JobOps& job,
        TFObserveStatusFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops
            ->get_status(get_handle(), job.get_handle(), completion, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_result(
        const ice::sonic::TF_JobOps& job,
        TFObserveResultFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops
            ->get_result(get_handle(), job.get_handle(), completion, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_history(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_transitions
    ) noexcept
    {
        ice::Status status;
        m_ops->get_history(
            get_handle(),
            job.get_handle(),
            out_transitions.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_metrics(const ice::sonic::TF_JobOps& job, const ice::sonic::TF_MapOps& out_metrics) noexcept
    {
        ice::Status status;
        m_ops->get_metrics(
            get_handle(),
            job.get_handle(),
            out_metrics.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_logs(const ice::sonic::TF_JobOps& job, const ice::sonic::TF_VectorOps& out_lines) noexcept
    {
        ice::Status status;
        m_ops->get_logs(get_handle(), job.get_handle(), out_lines.get_handle() status.get_handle());

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
