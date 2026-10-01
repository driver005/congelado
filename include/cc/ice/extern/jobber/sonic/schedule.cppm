// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/schedule.h"

export module cc_ice_extern_jobber_sonic:schedule;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ScheduleOps : public ice::sonic::Runtime<::TF_ScheduleOps, ::TF_Schedule>
{
public:
    template<typename Registry>
    TF_ScheduleOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ScheduleOps(
        Registry& registry,
        ::TF_Schedule* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ScheduleOps(const ::TF_ScheduleOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ScheduleOps(const ::TF_ScheduleOps* ops, ::TF_Schedule* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void add_dependency(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_JobOps& depends_on,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_dependency(
            get_handle(),
            job.get_handle(),
            depends_on.get_handle(),
            out_status.get_handle()
        );
    }

    void list_dependencies(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_job_ids,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_dependencies(
            get_handle(),
            job.get_handle(),
            out_job_ids.get_handle(),
            out_status.get_handle()
        );
    }

    void pause(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->pause(get_handle(), job.get_handle(), out_status.get_handle());
    }

    void resume(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->resume(get_handle(), job.get_handle(), out_status.get_handle());
    }

    void cancel(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->cancel(get_handle(), job.get_handle(), out_status.get_handle());
    }

    void stop(const ice::sonic::TF_JobOps& job, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->stop(get_handle(), job.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
