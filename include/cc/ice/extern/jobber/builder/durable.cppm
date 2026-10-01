// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/durable.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/durable.h"
#include "include/c/extern/jobber/job.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_jobber_builder:durable;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_DurableOps
{
public:
    explicit TF_DurableOps(
        const ::TF_JobOps* TF_JobOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_JobOps_ops = TF_JobOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_DurableOps(const TF_DurableOps&) = delete;
    TF_DurableOps& operator=(const TF_DurableOps&) = delete;

    static TF_DurableOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DurableOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DurableOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DurableOps*>(handle->plugin_data);
    }

    virtual ~TF_DurableOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void signal(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::String& signal_name,
        const ice::sonic::String& payload,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void checkpoint(
        const ice::sonic::TF_JobOps& job,
        TFDurableAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void restore_checkpoint(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Durable*)) noexcept
    {
        m_vtable = ::TF_DurableOps{
            .struct_size = TF_OFFSET_OF_END(::TF_DurableOps, restore_checkpoint),

            .create = create,
            .destroy =
                [](TF_Durable* handle) noexcept
            {
                auto& self = TF_DurableOps::from_handle(handle);
                self.destroy();
            },
            .signal =
                [](TF_Durable* durable,
                   TF_Job* job,
                   const TF_String* signal_name,
                   const TF_String* payload,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_DurableOps::from_handle(durable);
                self.signal(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    self.wrap(std::type_identity<ice::sonic::String>{}, signal_name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, payload),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .checkpoint =
                [](TF_Durable* durable,
                   TF_Job* job,
                   TFDurableAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_DurableOps::from_handle(durable);
                self.checkpoint(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .restore_checkpoint =
                [](TF_Durable* durable, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto& self = TF_DurableOps::from_handle(durable);
                self.restore_checkpoint(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
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

    const ::TF_DurableOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Durable& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_DurableOps*>(&m_vtable));
    }

private:
    ::TF_DurableOps m_vtable;
    ::TF_Durable m_handle;

    const ::TF_JobOps* m_TF_JobOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
