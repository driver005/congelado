// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/schedule.h"

export module cc_abi_sonic_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ScheduleOps : public ice::sonic::Runtime<TF_ScheduleOps, TF_ScheduleOps>
{
public:
    explicit TF_ScheduleOps(TF_ScheduleOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "jobber";

    [[nodiscard]] std::expected<void, ice::Status> add_dependency(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_JobOps& depends_on
    ) noexcept
    {
        ice::Status status;
        m_ops->add_dependency(
            get_handle(),
            job.get_handle(),
            depends_on.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_dependencies(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_job_ids
    ) noexcept
    {
        ice::Status status;
        m_ops->list_dependencies(
            get_handle(),
            job.get_handle(),
            out_job_ids.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> pause(const ice::sonic::TF_JobOps& job) noexcept
    {
        ice::Status status;
        m_ops->pause(get_handle(), job.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> resume(const ice::sonic::TF_JobOps& job) noexcept
    {
        ice::Status status;
        m_ops->resume(get_handle(), job.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> cancel(const ice::sonic::TF_JobOps& job) noexcept
    {
        ice::Status status;
        m_ops->cancel(get_handle(), job.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stop(const ice::sonic::TF_JobOps& job) noexcept
    {
        ice::Status status;
        m_ops->stop(get_handle(), job.get_handle() status.get_handle());

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
