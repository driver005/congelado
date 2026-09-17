// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"

export module cc_abi_sonic_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_JobOps : public ice::sonic::Runtime<TF_JobOps, TF_JobOps>
{
public:
    explicit TF_JobOps(TF_JobOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "jobber";

    [[nodiscard]] std::expected<void, ice::Status> execute(
        const ice::sonic::TF_StringOps& input,
        const ice::sonic::TF_StringOps& out_output
    ) noexcept
    {
        ice::Status status;
        m_ops->execute(
            get_handle(),
            input.get_handle(),
            out_output.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> resubmit() noexcept
    {
        ice::Status status;
        m_ops->resubmit(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    wait(int64_t timeout_ms, TFJobCompletionFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->wait(get_handle(), timeout_ms, completion, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_complete(TFJobCompletionFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_complete(get_handle(), completion, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_progress(TFJobProgressFn progress, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_progress(get_handle(), progress, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list(const ice::sonic::TF_MapOps& filters, const ice::sonic::TF_VectorOps& out_job_ids) noexcept
    {
        ice::Status status;
        m_ops->list(
            get_handle(),
            filters.get_handle(),
            out_job_ids.get_handle() status.get_handle()
        );

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
