// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/job/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/job/job.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_job;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Job
{
public:
    static Job* create(void* ctx) noexcept
    {
        return static_cast<Job*>(ctx);
    }

    template<typename HandleT>
    static Job* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Job*>(handle);
    }

    virtual ~Job() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    execute(const ice::sonic::String& input, const ice::sonic::String& out_output) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    submit(const ice::sonic::String& input, const TF_Job_Options* options) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> resubmit() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    wait(int64_t timeout_ms, TF_Job_ResultFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_complete(TF_Job_CompletionFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_progress(TF_Job_ProgressFn progress, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_status(TF_Job_StatusFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_result(TF_Job_ResultFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_history(const ice::sonic::Vector& out_transitions) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_metrics(const ice::sonic::Map& out_metrics) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_logs(const ice::sonic::Vector& out_lines) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_options(TF_Job_Options* out_options) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    update_options(const TF_Job_Options* options) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_priority(int priority) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_dependency(const ice::sonic::Job& depends_on) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_dependencies(const ice::sonic::Vector& out_job_ids) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> pause() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> resume() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> cancel() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    signal(const ice::sonic::String& signal_name, const ice::sonic::String& payload) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    checkpoint(TF_Job_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> restore_checkpoint() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    complete(const ice::sonic::String& node_ref, const ice::sonic::String& output) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    create_task(const ice::sonic::String& node_ref, const ice::sonic::String& input) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_task(const ice::sonic::String& node_ref) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_tasks(const ice::sonic::Vector& out_node_refs) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    cancel_task(const ice::sonic::String& node_ref) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list(const ice::sonic::Map& filters, const ice::sonic::Vector& out_job_ids) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> destroy_job() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Job* get_generic_vtable()
    {
        static TF_Job vtable = {
            .struct_size = TF_JOB_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Job::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Job::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .execute =
                [](void* plugin_context,
                   const TF_String_Handle* input,
                   TF_String_Handle* out_output,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(plugin_context);
                auto res = self->execute(
                    ice::sonic::String::wrap(input),
                    ice::sonic::String::wrap(out_output)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .submit =
                [](void* plugin_context,
                   const TF_String_Handle* input,
                   const TF_Job_Options* options,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(plugin_context);
                auto res = self->submit(ice::sonic::String::wrap(input), options);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .resubmit =
                [](TF_Job_Handle* job, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->resubmit();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .wait =
                [](TF_Job_Handle* job,
                   int64_t timeout_ms,
                   TF_Job_ResultFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->wait(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_complete =
                [](TF_Job_Handle* job, TF_Job_CompletionFn completion, void* user_data) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->on_complete(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_progress =
                [](TF_Job_Handle* job, TF_Job_ProgressFn progress, void* user_data) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->on_progress(progress, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_status =
                [](TF_Job_Handle* job,
                   TF_Job_StatusFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->get_status(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_result =
                [](TF_Job_Handle* job,
                   TF_Job_ResultFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->get_result(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_history =
                [](TF_Job_Handle* job,
                   TF_Vector_Handle* out_transitions,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->get_history(ice::sonic::Vector::wrap(out_transitions));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_metrics =
                [](TF_Job_Handle* job,
                   TF_Map_Handle* out_metrics,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->get_metrics(ice::sonic::Map::wrap(out_metrics));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_logs =
                [](TF_Job_Handle* job,
                   TF_Vector_Handle* out_lines,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->get_logs(ice::sonic::Vector::wrap(out_lines));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_options =
                [](TF_Job_Handle* job,
                   TF_Job_Options* out_options,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->get_options(out_options);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .update_options =
                [](TF_Job_Handle* job,
                   const TF_Job_Options* options,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->update_options(options);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_priority =
                [](TF_Job_Handle* job, int priority, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->set_priority(priority);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_dependency =
                [](TF_Job_Handle* job, TF_Job_Handle* depends_on, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->add_dependency(ice::sonic::Job::wrap(depends_on));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_dependencies =
                [](TF_Job_Handle* job,
                   TF_Vector_Handle* out_job_ids,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->list_dependencies(ice::sonic::Vector::wrap(out_job_ids));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pause =
                [](TF_Job_Handle* job, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->pause();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .resume =
                [](TF_Job_Handle* job, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->resume();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .cancel =
                [](TF_Job_Handle* job, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->cancel();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stop =
                [](TF_Job_Handle* job, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .signal =
                [](TF_Job_Handle* job,
                   const TF_String_Handle* signal_name,
                   const TF_String_Handle* payload,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->signal(
                    ice::sonic::String::wrap(signal_name),
                    ice::sonic::String::wrap(payload)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .checkpoint =
                [](TF_Job_Handle* job,
                   TF_Job_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->checkpoint(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .restore_checkpoint =
                [](TF_Job_Handle* job, TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->restore_checkpoint();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .complete =
                [](TF_Job_Handle* job,
                   const TF_String_Handle* node_ref,
                   const TF_String_Handle* output,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->complete(
                    ice::sonic::String::wrap(node_ref),
                    ice::sonic::String::wrap(output)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_task =
                [](TF_Job_Handle* parent,
                   const TF_String_Handle* node_ref,
                   const TF_String_Handle* input,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(parent);
                auto res = self->create_task(
                    ice::sonic::String::wrap(node_ref),
                    ice::sonic::String::wrap(input)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_task =
                [](TF_Job_Handle* parent,
                   const TF_String_Handle* node_ref,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(parent);
                auto res = self->get_task(ice::sonic::String::wrap(node_ref));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_tasks =
                [](TF_Job_Handle* parent,
                   TF_Vector_Handle* out_node_refs,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(parent);
                auto res = self->list_tasks(ice::sonic::Vector::wrap(out_node_refs));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .cancel_task =
                [](TF_Job_Handle* parent,
                   const TF_String_Handle* node_ref,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(parent);
                auto res = self->cancel_task(ice::sonic::String::wrap(node_ref));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list =
                [](void* plugin_context,
                   const TF_Map_Handle* filters,
                   TF_Vector_Handle* out_job_ids,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Job::create(plugin_context);
                auto res = self->list(
                    ice::sonic::Map::wrap(filters),
                    ice::sonic::Vector::wrap(out_job_ids)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .destroy_job =
                [](TF_Job_Handle* job) noexcept
            {
                auto* self = Job::create(job);
                auto res = self->destroy_job();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
