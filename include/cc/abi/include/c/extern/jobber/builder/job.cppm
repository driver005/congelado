// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"

export module cc_abi_builder_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_JobOps
{
public:
    static TF_JobOps* create(void* ctx) noexcept
    {
        return static_cast<TF_JobOps*>(ctx);
    }

    template<typename HandleT>
    static TF_JobOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_JobOps*>(handle->plugin_data);
    }

    virtual ~TF_JobOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> execute(
        const ice::sonic::TF_StringOps& input,
        const ice::sonic::TF_StringOps& out_output
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> resubmit() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    wait(int64_t timeout_ms, TFJobCompletionFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_complete(TFJobCompletionFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_progress(TFJobProgressFn progress, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> list(
        const ice::sonic::TF_MapOps& filters,
        const ice::sonic::TF_VectorOps& out_job_ids
    ) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_JobOps* get_generic_vtable()
    {
        static TF_JobOps vtable = {
            .struct_size = TF_JOB_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_JobOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_JobOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .execute =
                [](TF_Job* job,
                   const TF_String* input,
                   TF_String* out_output,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_JobOps::create(job);
                auto res = self->execute(
                    ice::sonic::TF_StringOps::wrap(input),
                    ice::sonic::TF_StringOps::wrap(out_output)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .resubmit =
                [](TF_Job* job, TF_Status* out_status) noexcept
            {
                auto* self = TF_JobOps::create(job);
                auto res = self->resubmit();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .wait =
                [](TF_Job* job,
                   int64_t timeout_ms,
                   TFJobCompletionFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_JobOps::create(job);
                auto res = self->wait(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .on_complete =
                [](TF_Job* job, TFJobCompletionFn completion, void* user_data) noexcept
            {
                auto* self = TF_JobOps::create(job);
                auto res = self->on_complete(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_progress =
                [](TF_Job* job, TFJobProgressFn progress, void* user_data) noexcept
            {
                auto* self = TF_JobOps::create(job);
                auto res = self->on_progress(progress, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list =
                [](TF_Job* job,
                   const TF_Map* filters,
                   TF_Vector* out_job_ids,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_JobOps::create(job);
                auto res = self->list(
                    ice::sonic::TF_MapOps::wrap(filters),
                    ice::sonic::TF_VectorOps::wrap(out_job_ids)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
