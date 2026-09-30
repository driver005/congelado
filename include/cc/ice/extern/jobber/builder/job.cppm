// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"

export module cc_ice_extern_jobber_builder:job;

import std;

export namespace ice::builder {

class TF_JobOps
{
public:
    TF_JobOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_JobOps(const TF_JobOps&) = delete;
    TF_JobOps& operator=(const TF_JobOps&) = delete;

    static TF_JobOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_JobOps*>(ctx);
    }

    template<typename HandleT>
    static TF_JobOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_JobOps*>(handle->plugin_data);
    }

    virtual ~TF_JobOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    execute(const ice::sonic::String& input, const ice::sonic::String& out_output) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> resubmit() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    wait(int64_t timeout_ms, TFJobCompletionFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_complete(TFJobCompletionFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_progress(TFJobProgressFn progress, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> list(
        const ice::sonic::TF_MapOps& filters,
        const ice::sonic::TF_VectorOps& out_job_ids
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_JobOps{
            .struct_size = TF_JOB_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_JobOps>{&TF_JobOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_JobOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .execute =
                [](TF_Job* job,
                   const TF_String* input,
                   TF_String* out_output,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_JobOps::from_handle(job).execute(
                    ice::sonic::String::wrap(input),
                    ice::sonic::String::wrap(out_output)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .resubmit =
                [](TF_Job* job, TF_Status* out_status) noexcept
            {
                auto res = TF_JobOps::from_handle(job).resubmit();
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
                auto res = TF_JobOps::from_handle(job).wait(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .on_complete =
                [](TF_Job* job, TFJobCompletionFn completion, void* user_data) noexcept
            {
                auto res = TF_JobOps::from_handle(job).on_complete(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_progress =
                [](TF_Job* job, TFJobProgressFn progress, void* user_data) noexcept
            {
                auto res = TF_JobOps::from_handle(job).on_progress(progress, user_data);
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
                auto res = TF_JobOps::from_handle(job).list(
                    ice::sonic::TF_MapOps::wrap(filters),
                    ice::sonic::TF_VectorOps::wrap(out_job_ids)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_JobOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Job& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_JobOps m_vtable;
    TF_Job m_handle;
};

} // namespace ice::builder
