// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/observe.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/observe.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_jobber_builder:observe;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ObserveOps
{
public:
    explicit TF_ObserveOps(
        const ::TF_JobOps* TF_JobOps_ops,
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_JobOps_ops = TF_JobOps_ops;
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_Status_ops = Status_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
    }

    TF_ObserveOps(const TF_ObserveOps&) = delete;
    TF_ObserveOps& operator=(const TF_ObserveOps&) = delete;

    static TF_ObserveOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ObserveOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ObserveOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ObserveOps*>(handle->plugin_data);
    }

    virtual ~TF_ObserveOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_status(
        const ice::sonic::TF_JobOps& job,
        TFObserveStatusFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_result(
        const ice::sonic::TF_JobOps& job,
        TFObserveResultFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_history(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_transitions,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_metrics(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_MapOps& out_metrics,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_logs(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_lines,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Observe*)) noexcept
    {
        m_vtable = ::TF_ObserveOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ObserveOps, get_logs),

            .create = create,
            .destroy =
                [](TF_Observe* handle) noexcept
            {
                auto& self = TF_ObserveOps::from_handle(handle);
                self.destroy();
            },
            .get_status =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TFObserveStatusFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ObserveOps::from_handle(observe);
                self.get_status(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_result =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TFObserveResultFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ObserveOps::from_handle(observe);
                self.get_result(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_history =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TF_Vector* out_transitions,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ObserveOps::from_handle(observe);
                self.get_history(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_transitions),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_metrics =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TF_Map* out_metrics,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ObserveOps::from_handle(observe);
                self.get_metrics(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_metrics),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_logs =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TF_Vector* out_lines,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ObserveOps::from_handle(observe);
                self.get_logs(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_lines),
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

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TF_ObserveOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Observe& get_handle() const noexcept
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
            const_cast<::TF_ObserveOps*>(&m_vtable)
        );
    }

private:
    ::TF_ObserveOps m_vtable;
    ::TF_Observe m_handle;

    const ::TF_JobOps* m_TF_JobOps_ops{nullptr};

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
