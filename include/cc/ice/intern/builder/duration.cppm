// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/duration.h"

export module cc_ice_intern_builder:duration;

import std;

export namespace ice::builder {

class TF_DurationOps
{
public:
    TF_DurationOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_DurationOps(const TF_DurationOps&) = delete;
    TF_DurationOps& operator=(const TF_DurationOps&) = delete;

    static TF_DurationOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DurationOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DurationOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DurationOps*>(handle->plugin_data);
    }

    virtual ~TF_DurationOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ticks(int64_t* out_ticks) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ratio_num(int64_t* out_num) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ratio_den(int64_t* out_den) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DurationOps{
            .struct_size = TF_DURATION_STRUCT_SIZE,
            .get_ticks =
                [](const TF_Duration* duration, int64_t* out_ticks) noexcept
            {
                auto res = TF_DurationOps::from_handle(duration).get_ticks(out_ticks);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ratio_num =
                [](const TF_Duration* duration, int64_t* out_num) noexcept
            {
                auto res = TF_DurationOps::from_handle(duration).get_ratio_num(out_num);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ratio_den =
                [](const TF_Duration* duration, int64_t* out_den) noexcept
            {
                auto res = TF_DurationOps::from_handle(duration).get_ratio_den(out_den);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_DurationOps>{&TF_DurationOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_DurationOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Duration& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DurationOps m_vtable;
    TF_Duration m_handle;
};

} // namespace ice::builder
