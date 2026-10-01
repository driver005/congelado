// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/schedule.h"
#include "include/c/intern/status.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_jobber_builder:schedule;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ScheduleOps
{
public:
    explicit TF_ScheduleOps(
        const ::TF_JobOps* TF_JobOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_JobOps_ops = TF_JobOps_ops;
        m_Status_ops = Status_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
    }

    TF_ScheduleOps(const TF_ScheduleOps&) = delete;
    TF_ScheduleOps& operator=(const TF_ScheduleOps&) = delete;

    static TF_ScheduleOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ScheduleOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ScheduleOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ScheduleOps*>(handle->plugin_data);
    }

    virtual ~TF_ScheduleOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void add_dependency(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_JobOps& depends_on,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void list_dependencies(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_job_ids,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    pause(const ice::sonic::TF_JobOps& job, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    resume(const ice::sonic::TF_JobOps& job, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    cancel(const ice::sonic::TF_JobOps& job, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    stop(const ice::sonic::TF_JobOps& job, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Schedule*)) noexcept
    {
        m_vtable = ::TF_ScheduleOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ScheduleOps, stop),

            .create = create,
            .destroy =
                [](TF_Schedule* handle) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(handle);
                self.destroy();
            },
            .add_dependency =
                [](TF_Schedule* schedule,
                   TF_Job* job,
                   TF_Job* depends_on,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(schedule);
                self.add_dependency(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, depends_on),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_dependencies =
                [](TF_Schedule* schedule,
                   TF_Job* job,
                   TF_Vector* out_job_ids,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(schedule);
                self.list_dependencies(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_job_ids),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .pause =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(schedule);
                self.pause(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .resume =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(schedule);
                self.resume(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .cancel =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(schedule);
                self.cancel(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .stop =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto& self = TF_ScheduleOps::from_handle(schedule);
                self.stop(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_JobOps
    wrap(std::type_identity<ice::sonic::TF_JobOps>, const ::TF_Job* handle) const noexcept
    {
        return ice::sonic::TF_JobOps{m_TF_JobOps_ops, const_cast<::TF_Job*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TF_ScheduleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Schedule& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ScheduleOps*>(&m_vtable));
    }

private:
    ::TF_ScheduleOps m_vtable;
    ::TF_Schedule m_handle;

    const ::TF_JobOps* m_TF_JobOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
