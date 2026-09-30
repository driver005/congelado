// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/durable.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/durable.h"

export module cc_ice_extern_jobber_builder:durable;

import std;

export namespace ice::builder {

class TF_DurableOps
{
public:
    TF_DurableOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status> signal(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::String& signal_name,
        const ice::sonic::String& payload
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> checkpoint(
        const ice::sonic::TF_JobOps& job,
        TFDurableAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    restore_checkpoint(const ice::sonic::TF_JobOps& job) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DurableOps{
            .struct_size = TF_DURABLE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_DurableOps>{&TF_DurableOps::from_handle(plugin_context)};
            },
            .signal =
                [](TF_Durable* durable,
                   TF_Job* job,
                   const TF_String* signal_name,
                   const TF_String* payload,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_DurableOps::from_handle(durable).signal(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::String::wrap(signal_name),
                    ice::sonic::String::wrap(payload)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .checkpoint =
                [](TF_Durable* durable,
                   TF_Job* job,
                   TFDurableAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_DurableOps::from_handle(durable)
                               .checkpoint(ice::sonic::TF_JobOps::wrap(job), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .restore_checkpoint =
                [](TF_Durable* durable, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto res = TF_DurableOps::from_handle(durable).restore_checkpoint(
                    ice::sonic::TF_JobOps::wrap(job)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_DurableOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Durable& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DurableOps m_vtable;
    TF_Durable m_handle;
};

} // namespace ice::builder
