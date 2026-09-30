// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/time_point.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/time_point.h"

export module cc_ice_intern_builder:time_point;

import std;

export namespace ice::builder {

class TF_TimePointOps
{
public:
    TF_TimePointOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_TimePointOps(const TF_TimePointOps&) = delete;
    TF_TimePointOps& operator=(const TF_TimePointOps&) = delete;

    static TF_TimePointOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_TimePointOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TimePointOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_TimePointOps*>(handle->plugin_data);
    }

    virtual ~TF_TimePointOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_duration_since_epoch(const ice::sonic::TF_DurationOps& out_duration) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_TimePointOps{
            .struct_size = TF_TIMEPOINT_STRUCT_SIZE,
            .get_duration_since_epoch =
                [](const TF_TimePoint* time_point, TF_Duration* out_duration) noexcept
            {
                auto res =
                    TF_TimePointOps::from_handle(time_point)
                        .get_duration_since_epoch(ice::sonic::TF_DurationOps::wrap(out_duration));
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_TimePointOps>{&TF_TimePointOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_TimePointOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_TimePoint& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_TimePointOps m_vtable;
    TF_TimePoint m_handle;
};

} // namespace ice::builder
