// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/task.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/task.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_jobber_builder:task;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_TaskOps
{
public:
    explicit TF_TaskOps(
        const ::TF_JobOps* TF_JobOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_JobOps_ops = TF_JobOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void complete(
        const ice::sonic::String& node_ref,
        const ice::sonic::String& output,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::String& input,
        const ice::sonic::TF_JobOps& out_child,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::TF_JobOps& out_child
    ) noexcept = 0;
    virtual void list_tasks(
        const ice::sonic::TF_VectorOps& out_node_refs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void cancel_task(
        const ice::sonic::String& node_ref,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Task*)) noexcept
    {
        m_vtable = ::TF_TaskOps{
            .struct_size = TF_OFFSET_OF_END(::TF_TaskOps, cancel_task),

            .create = create,
            .destroy =
                [](TF_Task* handle) noexcept
            {
                auto& self = TF_TaskOps::from_handle(handle);
                self.destroy();
            },
            .complete =
                [](TF_Task* task,
                   const TF_String* node_ref,
                   const TF_String* output,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_TaskOps::from_handle(task);
                self.complete(
                    self.wrap(std::type_identity<ice::sonic::String>{}, node_ref),
                    self.wrap(std::type_identity<ice::sonic::String>{}, output),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_task =
                [](TF_Task* task,
                   const TF_String* node_ref,
                   const TF_String* input,
                   TF_Job* out_child,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_TaskOps::from_handle(task);
                self.create_task(
                    self.wrap(std::type_identity<ice::sonic::String>{}, node_ref),
                    self.wrap(std::type_identity<ice::sonic::String>{}, input),
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, out_child),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_task =
                [](TF_Task* task, const TF_String* node_ref, TF_Job* out_child) noexcept
            {
                auto& self = TF_TaskOps::from_handle(task);
                self.get_task(
                    self.wrap(std::type_identity<ice::sonic::String>{}, node_ref),
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, out_child)
                );
            },
            .list_tasks =
                [](TF_Task* task, TF_Vector* out_node_refs, TF_Status* out_status) noexcept
            {
                auto& self = TF_TaskOps::from_handle(task);
                self.list_tasks(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_node_refs),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .cancel_task =
                [](TF_Task* task, const TF_String* node_ref, TF_Status* out_status) noexcept
            {
                auto& self = TF_TaskOps::from_handle(task);
                self.cancel_task(
                    self.wrap(std::type_identity<ice::sonic::String>{}, node_ref),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_JobOps
    wrap(std::type_identity<ice::sonic::TF_JobOps>, const ::TF_Job* handle) const noexcept
    {
        return ice::sonic::TF_JobOps{m_TF_JobOps_ops, const_cast<::TF_Job*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TF_TaskOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Task& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_TaskOps*>(&m_vtable)
        );
    }

private:
    ::TF_TaskOps m_vtable;
    ::TF_Task m_handle;

    const ::TF_JobOps* m_TF_JobOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
