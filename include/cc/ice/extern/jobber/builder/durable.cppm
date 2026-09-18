// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/durable.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/durable.h"

export module cc_ice_builder_jobber:durable;

import std;

export namespace ice::builder {

class TF_DurableOps
{
public:
    static TF_DurableOps* create(void* ctx) noexcept
    {
        return static_cast<TF_DurableOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DurableOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_DurableOps*>(handle->plugin_data);
    }

    virtual ~TF_DurableOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> signal(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_StringOps& signal_name,
        const ice::sonic::TF_StringOps& payload
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> checkpoint(
        const ice::sonic::TF_JobOps& job,
        TFDurableAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    restore_checkpoint(const ice::sonic::TF_JobOps& job) noexcept = 0;

    static TF_DurableOps* get_generic_vtable()
    {
        static TF_DurableOps vtable = {
            .struct_size = TF_DURABLE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_DurableOps::create(plugin_context);
            },
            .signal =
                [](TF_Durable* durable,
                   TF_Job* job,
                   const TF_String* signal_name,
                   const TF_String* payload,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_DurableOps::create(durable);
                auto res = self->signal(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::TF_StringOps::wrap(signal_name),
                    ice::sonic::TF_StringOps::wrap(payload)
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
                auto* self = TF_DurableOps::create(durable);
                auto res =
                    self->checkpoint(ice::sonic::TF_JobOps::wrap(job), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .restore_checkpoint =
                [](TF_Durable* durable, TF_Job* job, TF_Status* out_status) noexcept
            {
                auto* self = TF_DurableOps::create(durable);
                auto res = self->restore_checkpoint(ice::sonic::TF_JobOps::wrap(job));
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
