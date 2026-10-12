// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/observe.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/observe.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_jobber_builder:observe;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Observe
{
public:
    explicit Observe(
        const ::TF_JobOps* Job_ops,
        const ::TF_MapOps* Map_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_VectorOps* Vector_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Job_ops = Job_ops;
        m_Map_ops = Map_ops;
        m_Status_ops = Status_ops;
        m_Vector_ops = Vector_ops;
    }

    Observe(const Observe&) = delete;
    Observe& operator=(const Observe&) = delete;

    static Observe& from_handle(void* ctx) noexcept
    {
        return *static_cast<Observe*>(ctx);
    }

    template<typename HandleT>
    static Observe& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Observe*>(handle->plugin_data);
    }

    virtual ~Observe() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_status(
        const ice::sonic::Job& job,
        TFObserveStatusFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_result(
        const ice::sonic::Job& job,
        TFObserveResultFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_history(
        const ice::sonic::Job& job,
        const ice::sonic::Vector& out_transitions,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_metrics(
        const ice::sonic::Job& job,
        const ice::sonic::Map& out_metrics,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_logs(
        const ice::sonic::Job& job,
        const ice::sonic::Vector& out_lines,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Observe*)) noexcept
    {
        m_vtable = ::TF_ObserveOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ObserveOps, get_logs),

            .create = create,
            .destroy =
                [](::TF_Observe* handle) noexcept
            {
                auto& self = Observe::from_handle(handle);
                self.destroy();
            },
            .get_status =
                [](::TF_Observe* observe,
                   ::TF_Job* job,
                   TFObserveStatusFn completion,
                   void* user_data,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Observe::from_handle(observe);
                self.get_status(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_result =
                [](::TF_Observe* observe,
                   ::TF_Job* job,
                   TFObserveResultFn completion,
                   void* user_data,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Observe::from_handle(observe);
                self.get_result(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_history =
                [](::TF_Observe* observe,
                   ::TF_Job* job,
                   ::TF_Vector* out_transitions,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Observe::from_handle(observe);
                self.get_history(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_transitions),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_metrics =
                [](::TF_Observe* observe,
                   ::TF_Job* job,
                   ::TF_Map* out_metrics,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Observe::from_handle(observe);
                self.get_metrics(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Map>{}, out_metrics),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_logs =
                [](::TF_Observe* observe,
                   ::TF_Job* job,
                   ::TF_Vector* out_lines,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Observe::from_handle(observe);
                self.get_logs(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_lines),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Job wrap(std::type_identity<ice::sonic::Job>, const ::TF_Job* handle) const noexcept
    {
        return ice::sonic::Job{m_Job_ops, const_cast<::TF_Job*>(handle)};
    }

    ice::sonic::Map wrap(std::type_identity<ice::sonic::Map>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::Map{m_Map_ops, const_cast<::TF_Map*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::Vector
    wrap(std::type_identity<ice::sonic::Vector>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::Vector{m_Vector_ops, const_cast<::TF_Vector*>(handle)};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ObserveOps*>(&m_vtable));
    }

private:
    ::TF_ObserveOps m_vtable;
    ::TF_Observe m_handle;

    const ::TF_JobOps* m_Job_ops{nullptr};

    const ::TF_MapOps* m_Map_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_VectorOps* m_Vector_ops{nullptr};
};

} // namespace ice::builder
