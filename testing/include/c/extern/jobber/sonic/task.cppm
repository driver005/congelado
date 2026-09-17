// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/task.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/task.h"

export module cc_abi_sonic_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TaskOps : public ice::sonic::Runtime<TF_TaskOps, TF_TaskOps>
{
public:
    explicit TF_TaskOps(TF_TaskOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "jobber";

    [[nodiscard]] std::expected<void, ice::Status> complete(
        const ice::sonic::TF_StringOps& node_ref,
        const ice::sonic::TF_StringOps& output
    ) noexcept
    {
        ice::Status status;
        m_ops->complete(
            get_handle(),
            node_ref.get_handle(),
            output.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> create_task(
        const ice::sonic::TF_StringOps& node_ref,
        const ice::sonic::TF_StringOps& input,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept
    {
        ice::Status status;
        m_ops->create_task(
            get_handle(),
            node_ref.get_handle(),
            input.get_handle(),
            out_child.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_task(
        const ice::sonic::TF_StringOps& node_ref,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept
    {
        ice::Status status;
        m_ops->get_task(
            get_handle(),
            node_ref.get_handle(),
            out_child.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_tasks(const ice::sonic::TF_VectorOps& out_node_refs) noexcept
    {
        ice::Status status;
        m_ops->list_tasks(get_handle(), out_node_refs.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    cancel_task(const ice::sonic::TF_StringOps& node_ref) noexcept
    {
        ice::Status status;
        m_ops->cancel_task(get_handle(), node_ref.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
