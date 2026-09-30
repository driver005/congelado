// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/task.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/task.h"

export module cc_ice_extern_jobber_builder:task;

import std;

export namespace ice::builder {

class TF_TaskOps
{
public:
    TF_TaskOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_TaskOps(const TF_TaskOps&) = delete;
    TF_TaskOps& operator=(const TF_TaskOps&) = delete;

    static TF_TaskOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_TaskOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TaskOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_TaskOps*>(handle->plugin_data);
    }

    virtual ~TF_TaskOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    complete(const ice::sonic::String& node_ref, const ice::sonic::String& output) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::String& input,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_tasks(const ice::sonic::TF_VectorOps& out_node_refs) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    cancel_task(const ice::sonic::String& node_ref) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_TaskOps{
            .struct_size = TF_TASK_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_TaskOps>{&TF_TaskOps::from_handle(plugin_context)};
            },
            .complete =
                [](TF_Task* task,
                   const TF_String* node_ref,
                   const TF_String* output,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_TaskOps::from_handle(task).complete(
                    ice::sonic::String::wrap(node_ref),
                    ice::sonic::String::wrap(output)
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
                auto res = TF_TaskOps::from_handle(task).create_task(
                    ice::sonic::String::wrap(node_ref),
                    ice::sonic::String::wrap(input),
                    ice::sonic::TF_JobOps::wrap(out_child)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_task =
                [](TF_Task* task, const TF_String* node_ref, TF_Job* out_child) noexcept
            {
                auto res = TF_TaskOps::from_handle(task).get_task(
                    ice::sonic::String::wrap(node_ref),
                    ice::sonic::TF_JobOps::wrap(out_child)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_tasks =
                [](TF_Task* task, TF_Vector* out_node_refs, TF_Status* out_status) noexcept
            {
                auto res = TF_TaskOps::from_handle(task).list_tasks(
                    ice::sonic::TF_VectorOps::wrap(out_node_refs)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .cancel_task =
                [](TF_Task* task, const TF_String* node_ref, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_TaskOps::from_handle(task).cancel_task(ice::sonic::String::wrap(node_ref));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_TaskOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Task& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_TaskOps m_vtable;
    TF_Task m_handle;
};

} // namespace ice::builder
