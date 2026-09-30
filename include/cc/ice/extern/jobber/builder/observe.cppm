// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/observe.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/observe.h"

export module cc_ice_extern_jobber_builder:observe;

import std;

export namespace ice::builder {

class TF_ObserveOps
{
public:
    TF_ObserveOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_status(
        const ice::sonic::TF_JobOps& job,
        TFObserveStatusFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_result(
        const ice::sonic::TF_JobOps& job,
        TFObserveResultFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_history(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_transitions
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_metrics(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_MapOps& out_metrics
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_logs(
        const ice::sonic::TF_JobOps& job,
        const ice::sonic::TF_VectorOps& out_lines
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ObserveOps{
            .struct_size = TF_OBSERVE_STRUCT_SIZE,
            .destroy =
                [](TF_Observe* observe) noexcept
            {
                TF_ObserveOps::from_handle(observe).destroy();
            },
            .get_status =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TFObserveStatusFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ObserveOps::from_handle(observe)
                               .get_status(ice::sonic::TF_JobOps::wrap(job), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_result =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TFObserveResultFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ObserveOps::from_handle(observe)
                               .get_result(ice::sonic::TF_JobOps::wrap(job), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_history =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TF_Vector* out_transitions,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ObserveOps::from_handle(observe).get_history(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::TF_VectorOps::wrap(out_transitions)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_metrics =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TF_Map* out_metrics,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ObserveOps::from_handle(observe).get_metrics(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::TF_MapOps::wrap(out_metrics)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_logs =
                [](TF_Observe* observe,
                   TF_Job* job,
                   TF_Vector* out_lines,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ObserveOps::from_handle(observe).get_logs(
                    ice::sonic::TF_JobOps::wrap(job),
                    ice::sonic::TF_VectorOps::wrap(out_lines)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_ObserveOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Observe& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ObserveOps m_vtable;
    TF_Observe m_handle;
};

} // namespace ice::builder
