// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/schedule.h"

export module cc_ice_extern_jobber_builder:schedule;

import std;

export namespace ice::builder {

class TF_ScheduleOps
{
public:
    TF_ScheduleOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> add_dependency(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_JobOps& depends_on
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> list_dependencies(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_job_ids
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    pause(const ice::sonic::TF_JobOps& job) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    resume(const ice::sonic::TF_JobOps& job) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    cancel(const ice::sonic::TF_JobOps& job) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    stop(const ice::sonic::TF_JobOps& job) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ScheduleOps{
            .struct_size = TF_SCHEDULE_STRUCT_SIZE,
            .destroy =
                [](TF_Schedule* schedule) noexcept
            {
                TF_ScheduleOps::from_handle(schedule).destroy();
            },
            .add_dependency =
                [](TF_Schedule* schedule,
                   TF_Job* job,
                   TF_Job* depends_on,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ScheduleOps::from_handle(schedule).add_dependency(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::TF_JobOps::wrap(depends_on)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_dependencies =
                [](TF_Schedule* schedule,
                   TF_Job* job,
                   TF_Vector* out_job_ids,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ScheduleOps::from_handle(schedule).list_dependencies(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::TF_VectorOps::wrap(out_job_ids)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .pause =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_ScheduleOps::from_handle(schedule).pause(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .resume =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_ScheduleOps::from_handle(schedule).resume(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .cancel =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_ScheduleOps::from_handle(schedule).cancel(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_ScheduleOps::from_handle(schedule).stop(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_ScheduleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Schedule& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ScheduleOps m_vtable;
    TF_Schedule m_handle;
};

} // namespace ice::builder
