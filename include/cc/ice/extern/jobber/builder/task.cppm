// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/task.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/task.h"

export module cc_ice_builder_jobber:task;

import std;

export namespace ice::builder {

class TF_TaskOps
{
public:
    static TF_TaskOps* create(void* ctx) noexcept
    {
        return static_cast<TF_TaskOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TaskOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_TaskOps*>(handle->plugin_data);
    }

    virtual ~TF_TaskOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> complete(
        const ice::sonic::TF_StringOps& node_ref,
        const ice::sonic::TF_StringOps& output
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_task(
        const ice::sonic::TF_StringOps& node_ref,
        const ice::sonic::TF_StringOps& input,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_task(
        const ice::sonic::TF_StringOps& node_ref,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_tasks(const ice::sonic::TF_VectorOps& out_node_refs) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    cancel_task(const ice::sonic::TF_StringOps& node_ref) noexcept = 0;

    static TF_TaskOps* get_generic_vtable()
    {
        static TF_TaskOps vtable = {
            .struct_size = TF_TASK_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_TaskOps::create(plugin_context);
            },
            .complete =
                [](TF_Task* task,
                   const TF_String* node_ref,
                   const TF_String* output,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_TaskOps::create(task);
                auto res = self->complete(
                    ice::sonic::TF_StringOps::wrap(node_ref),
                    ice::sonic::TF_StringOps::wrap(output)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_task =
                [](TF_Task* task,
                   const TF_String* node_ref,
                   const TF_String* input,
                   TF_Job* out_child,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_TaskOps::create(task);
                auto res = self->create_task(
                    ice::sonic::TF_StringOps::wrap(node_ref),
                    ice::sonic::TF_StringOps::wrap(input),
                    ice::sonic::TF_JobOps::wrap(out_child)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_task =
                [](TF_Task* task, const TF_String* node_ref, TF_Job* out_child) noexcept
            {
                auto* self = TF_TaskOps::create(task);
                auto res = self->get_task(
                    ice::sonic::TF_StringOps::wrap(node_ref),
                    ice::sonic::TF_JobOps::wrap(out_child)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_tasks =
                [](TF_Task* task, TF_Vector* out_node_refs, TF_Status* out_status) noexcept
            {
                auto* self = TF_TaskOps::create(task);
                auto res = self->list_tasks(ice::sonic::TF_VectorOps::wrap(out_node_refs));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .cancel_task =
                [](TF_Task* task, const TF_String* node_ref, TF_Status* out_status) noexcept
            {
                auto* self = TF_TaskOps::create(task);
                auto res = self->cancel_task(ice::sonic::TF_StringOps::wrap(node_ref));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
