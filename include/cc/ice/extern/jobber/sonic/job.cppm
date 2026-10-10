// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_jobber_sonic:job;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_JobOps : public ice::sonic::Runtime<::TF_JobOps, ::TF_Job>
{
public:
    TF_JobOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_JobOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Job* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_JobOps(const ::TF_JobOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_JobOps(const ::TF_JobOps* ops, ::TF_Job* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void execute(
        const ice::sonic::String& input,
        const ice::sonic::String& out_output,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->execute(
            get_handle(),
            input.get_handle(),
            out_output.get_handle(),
            out_status.get_handle()
        );
    }

    void resubmit(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->resubmit(get_handle(), out_status.get_handle());
    }

    void wait(
        int64_t timeout_ms,
        TFJobCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->wait(get_handle(), timeout_ms, completion, user_data, out_status.get_handle());
    }

    void on_complete(TFJobCompletionFn completion, void* user_data) const noexcept
    {
        m_ops->on_complete(get_handle(), completion, user_data);
    }

    void on_progress(TFJobProgressFn progress, void* user_data) const noexcept
    {
        m_ops->on_progress(get_handle(), progress, user_data);
    }

    void list(
        const ice::sonic::TF_MapOps& filters,
        const ice::sonic::TF_VectorOps& out_job_ids,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list(
            get_handle(),
            filters.get_handle(),
            out_job_ids.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
