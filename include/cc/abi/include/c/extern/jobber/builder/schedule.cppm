// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/schedule.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/schedule.h"

export module cc_abi_builder_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_ScheduleOps
{
public:
    static TF_ScheduleOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ScheduleOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ScheduleOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ScheduleOps*>(handle->plugin_data);
    }

    virtual ~TF_ScheduleOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> add_dependency(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_JobOps& depends_on
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> list_dependencies(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_job_ids
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    pause(const ice::sonic::TF_JobOps& job) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    resume(const ice::sonic::TF_JobOps& job) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    cancel(const ice::sonic::TF_JobOps& job) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    stop(const ice::sonic::TF_JobOps& job) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_ScheduleOps* get_generic_vtable()
    {
        static TF_ScheduleOps vtable = {
            .struct_size = TF_SCHEDULE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ScheduleOps::create(plugin_context);
            },
            .add_dependency =
                [](TF_Schedule* schedule,
                   TF_Job* job,
                   TF_Job* depends_on,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ScheduleOps::create(schedule);
                auto res = self->add_dependency(
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
                auto* self = TF_ScheduleOps::create(schedule);
                auto res = self->list_dependencies(
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
                auto* self = TF_ScheduleOps::create(schedule);
                auto res = self->pause(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .resume =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto* self = TF_ScheduleOps::create(schedule);
                auto res = self->resume(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .cancel =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto* self = TF_ScheduleOps::create(schedule);
                auto res = self->cancel(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop =
                [](TF_Schedule* schedule, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto* self = TF_ScheduleOps::create(schedule);
                auto res = self->stop(ice::sonic::TF_JobOps::wrap(job));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
