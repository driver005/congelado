// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/durable.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/durable.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_jobber_sonic:durable;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Durable : public ice::sonic::Runtime<::TF_DurableOps, ::TF_Durable>
{
public:
    Durable(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    Durable(
        const ice::sonic::Registration& registry,
        const ::TF_Durable& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit Durable(const ::TF_DurableOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Durable(const ::TF_DurableOps* ops, const ::TF_Durable* handle) noexcept :
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

    void signal(
        const ice::sonic::Job& job,
        const ice::sonic::String& signal_name,
        const ice::sonic::String& payload,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->signal(
            get_handle(),
            job.get_handle(),
            signal_name.get_handle(),
            payload.get_handle(),
            out_status.get_handle()
        );
    }

    void checkpoint(
        const ice::sonic::Job& job,
        TFDurableAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->checkpoint(
            get_handle(),
            job.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void restore_checkpoint(
        const ice::sonic::Job& job,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->restore_checkpoint(get_handle(), job.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
