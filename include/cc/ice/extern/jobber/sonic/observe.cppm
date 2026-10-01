// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/observe.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/observe.h"

export module cc_ice_extern_jobber_sonic:observe;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ObserveOps : public ice::sonic::Runtime<::TF_ObserveOps, ::TF_Observe>
{
public:
    template<typename Registry>
    TF_ObserveOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ObserveOps(
        Registry& registry,
        ::TF_Observe* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ObserveOps(const ::TF_ObserveOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ObserveOps(const ::TF_ObserveOps* ops, ::TF_Observe* handle) noexcept :
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

    void get_status(
        const ice::sonic::TF_JobOps& job,
        TFObserveStatusFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_status(
            get_handle(),
            job.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void get_result(
        const ice::sonic::TF_JobOps& job,
        TFObserveResultFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_result(
            get_handle(),
            job.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void get_history(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_transitions,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_history(
            get_handle(),
            job.get_handle(),
            out_transitions.get_handle(),
            out_status.get_handle()
        );
    }

    void get_metrics(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_MapOps& out_metrics,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_metrics(
            get_handle(),
            job.get_handle(),
            out_metrics.get_handle(),
            out_status.get_handle()
        );
    }

    void get_logs(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_lines,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_logs(
            get_handle(),
            job.get_handle(),
            out_lines.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
