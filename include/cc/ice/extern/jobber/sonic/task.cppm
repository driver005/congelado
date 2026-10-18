// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/task.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/task.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_jobber_sonic:task;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Task : public ice::sonic::Runtime<::TF_TaskOps, ::TF_Task>
{
public:
    Task(
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

    Task(
        const ice::sonic::Registration& registry,
        const ::TF_Task& handle,
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

    explicit Task(const ::TF_TaskOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Task(const ::TF_TaskOps* ops, const ::TF_Task* handle) noexcept :
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

    void complete(
        const ice::sonic::String& node_ref,
        const ice::sonic::String& output,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->complete(
            get_handle(),
            node_ref.get_handle(),
            output.get_handle(),
            out_status.get_handle()
        );
    }

    void create_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::String& input,
        const ice::sonic::Job& out_child,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_task(
            get_handle(),
            node_ref.get_handle(),
            input.get_handle(),
            out_child.get_handle(),
            out_status.get_handle()
        );
    }

    void get_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::Job& out_child
    ) const noexcept
    {
        m_ops->get_task(get_handle(), node_ref.get_handle(), out_child.get_handle());
    }

    void list_tasks(
        const ice::sonic::Vector& out_node_refs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_tasks(get_handle(), out_node_refs.get_handle(), out_status.get_handle());
    }

    void cancel_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->cancel_task(get_handle(), node_ref.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
