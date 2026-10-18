// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_jobber_builder:job;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Job
{
public:
    explicit Job(
        const ::TF_MapOps* Map_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* Vector_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Map_ops = Map_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_Vector_ops = Vector_ops;
    }

    Job(const Job&) = delete;
    Job& operator=(const Job&) = delete;

    static Job& from_handle(void* ctx) noexcept
    {
        return *static_cast<Job*>(ctx);
    }

    template<typename HandleT>
    static Job& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Job*>(handle->plugin_data);
    }

    virtual ~Job() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void execute(
        const ice::sonic::String& input,
        const ice::sonic::String& out_output,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void resubmit(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void wait(
        int64_t timeout_ms,
        TFJobCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void on_complete(TFJobCompletionFn completion, void* user_data) noexcept = 0;
    virtual void on_progress(TFJobProgressFn progress, void* user_data) noexcept = 0;
    virtual void list(
        const ice::sonic::Map& filters,
        const ice::sonic::Vector& out_job_ids,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Job*)) noexcept
    {
        m_vtable = ::TF_JobOps{
            .struct_size = TF_OFFSET_OF_END(::TF_JobOps, list),

            .create = create,
            .destroy =
                [](::TF_Job* handle) noexcept
            {
                auto& self = Job::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TF_Job* job, ::TF_String* out_name) noexcept
            {
                auto& self = Job::from_handle(job);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .execute =
                [](::TF_Job* job,
                   const ::TF_String* input,
                   ::TF_String* out_output,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Job::from_handle(job);
                self.execute(
                    self.wrap(std::type_identity<ice::sonic::String>{}, input),
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_output),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .resubmit =
                [](::TF_Job* job, ::TF_Status* out_status) noexcept
            {
                auto& self = Job::from_handle(job);
                self.resubmit(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .wait =
                [](::TF_Job* job,
                   int64_t timeout_ms,
                   TFJobCompletionFn completion,
                   void* user_data,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Job::from_handle(job);
                self.wait(
                    timeout_ms,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .on_complete =
                [](::TF_Job* job, TFJobCompletionFn completion, void* user_data) noexcept
            {
                auto& self = Job::from_handle(job);
                self.on_complete(completion, user_data);
            },
            .on_progress =
                [](::TF_Job* job, TFJobProgressFn progress, void* user_data) noexcept
            {
                auto& self = Job::from_handle(job);
                self.on_progress(progress, user_data);
            },
            .list =
                [](::TF_Job* job,
                   const ::TF_Map* filters,
                   ::TF_Vector* out_job_ids,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Job::from_handle(job);
                self.list(
                    self.wrap(std::type_identity<ice::sonic::Map>{}, filters),
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_job_ids),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Map wrap(std::type_identity<ice::sonic::Map>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::Map{m_Map_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    ice::sonic::Vector
    wrap(std::type_identity<ice::sonic::Vector>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::Vector{m_Vector_ops, handle};
    }

    const ::TF_JobOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Job& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_JobOps*>(&m_vtable));
    }

private:
    ::TF_JobOps m_vtable;
    ::TF_Job m_handle;

    const ::TF_MapOps* m_Map_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_Vector_ops{nullptr};
};

} // namespace ice::builder
