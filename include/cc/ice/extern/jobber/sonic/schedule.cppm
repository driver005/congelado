// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/schedule.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_jobber_sonic:schedule;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Schedule : public ice::sonic::Runtime<::TF_ScheduleOps, ::TF_Schedule>
{
public:
    Schedule(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    Schedule(
        const ice::sonic::Registration& registry,
        const ::TF_Schedule& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit Schedule(const ::TF_ScheduleOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Schedule(const ::TF_ScheduleOps* ops, const ::TF_Schedule* handle) noexcept :
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
        const ice::sonic::Job& job,
        const ice::sonic::Job& depends_on,
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
        const ice::sonic::Job& job,
        const ice::sonic::Vector& out_job_ids,
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

    void pause(const ice::sonic::Job& job, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->pause(get_handle(), job.get_handle(), out_status.get_handle());
    }

    void resume(const ice::sonic::Job& job, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->resume(get_handle(), job.get_handle(), out_status.get_handle());
    }

    void cancel(const ice::sonic::Job& job, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->cancel(get_handle(), job.get_handle(), out_status.get_handle());
    }

    void stop(const ice::sonic::Job& job, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->stop(get_handle(), job.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
