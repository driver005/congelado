// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/schedule.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_jobber_builder:schedule;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Schedule
{
public:
    explicit Schedule(
        const ::TF_JobOps* Job_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_VectorOps* Vector_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Job_ops = Job_ops;
        m_Status_ops = Status_ops;
        m_Vector_ops = Vector_ops;
    }

    Schedule(const Schedule&) = delete;
    Schedule& operator=(const Schedule&) = delete;

    static Schedule& from_handle(void* ctx) noexcept
    {
        return *static_cast<Schedule*>(ctx);
    }

    template<typename HandleT>
    static Schedule& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Schedule*>(handle->plugin_data);
    }

    virtual ~Schedule() = default;
    virtual void destroy() noexcept = 0;
    virtual void add_dependency(
        const ice::sonic::Job& job,
        const ice::sonic::Job& depends_on,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void list_dependencies(
        const ice::sonic::Job& job,
        const ice::sonic::Vector& out_job_ids,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    pause(const ice::sonic::Job& job, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    resume(const ice::sonic::Job& job, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    cancel(const ice::sonic::Job& job, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    stop(const ice::sonic::Job& job, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Schedule*)) noexcept
    {
        m_vtable = ::TF_ScheduleOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ScheduleOps, stop),

            .create = create,
            .destroy =
                [](::TF_Schedule* handle) noexcept
            {
                auto& self = Schedule::from_handle(handle);
                self.destroy();
            },
            .add_dependency =
                [](::TF_Schedule* schedule,
                   ::TF_Job* job,
                   ::TF_Job* depends_on,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Schedule::from_handle(schedule);
                self.add_dependency(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Job>{}, depends_on),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_dependencies =
                [](::TF_Schedule* schedule,
                   ::TF_Job* job,
                   ::TF_Vector* out_job_ids,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Schedule::from_handle(schedule);
                self.list_dependencies(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_job_ids),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .pause =
                [](::TF_Schedule* schedule, ::TF_Job* job, ::TF_Status* out_status) noexcept
            {
                auto& self = Schedule::from_handle(schedule);
                self.pause(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .resume =
                [](::TF_Schedule* schedule, ::TF_Job* job, ::TF_Status* out_status) noexcept
            {
                auto& self = Schedule::from_handle(schedule);
                self.resume(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .cancel =
                [](::TF_Schedule* schedule, ::TF_Job* job, ::TF_Status* out_status) noexcept
            {
                auto& self = Schedule::from_handle(schedule);
                self.cancel(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .stop =
                [](::TF_Schedule* schedule, ::TF_Job* job, ::TF_Status* out_status) noexcept
            {
                auto& self = Schedule::from_handle(schedule);
                self.stop(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Job wrap(std::type_identity<ice::sonic::Job>, const ::TF_Job* handle) const noexcept
    {
        return ice::sonic::Job{m_Job_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::Vector
    wrap(std::type_identity<ice::sonic::Vector>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::Vector{m_Vector_ops, handle};
    }

    const ::TF_ScheduleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Schedule& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ScheduleOps*>(&m_vtable));
    }

private:
    ::TF_ScheduleOps m_vtable;
    ::TF_Schedule m_handle;

    const ::TF_JobOps* m_Job_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_VectorOps* m_Vector_ops{nullptr};
};

} // namespace ice::builder
