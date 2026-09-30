// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"

export module cc_ice_extern_jobber_sonic:job;

import std;
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

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    execute(const ice::sonic::String& input, const ice::sonic::String& out_output) noexcept
    {
        ice::sonic::Status status;
        m_ops->execute(
            get_handle(),
            input.get_handle(),
            out_output.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> resubmit() noexcept
    {
        ice::sonic::Status status;
        m_ops->resubmit(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    wait(int64_t timeout_ms, TFJobCompletionFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->wait(get_handle(), timeout_ms, completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void on_complete(TFJobCompletionFn completion, void* user_data) noexcept
    {
        m_ops->on_complete(get_handle(), completion, user_data);
    }

    void on_progress(TFJobProgressFn progress, void* user_data) noexcept
    {
        m_ops->on_progress(get_handle(), progress, user_data);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list(const ice::sonic::TF_MapOps& filters, const ice::sonic::TF_VectorOps& out_job_ids) noexcept
    {
        ice::sonic::Status status;
        m_ops->list(
            get_handle(),
            filters.get_handle(),
            out_job_ids.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
