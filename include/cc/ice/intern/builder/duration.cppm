// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/duration.h"

export module cc_ice_builder_intern:duration;

import std;

export namespace ice::builder {

class TF_DurationOps
{
public:
    static TF_DurationOps* create(void* ctx) noexcept
    {
        return static_cast<TF_DurationOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DurationOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_DurationOps*>(handle->plugin_data);
    }

    virtual ~TF_DurationOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ticks(int64_t* out_ticks) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ratio_num(int64_t* out_num) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ratio_den(int64_t* out_den) noexcept = 0;

    static TF_DurationOps* get_generic_vtable()
    {
        static TF_DurationOps vtable = {
            .struct_size = TF_DURATION_STRUCT_SIZE,
            .get_ticks =
                [](const TF_Duration* duration, int64_t* out_ticks) noexcept
            {
                auto* self = TF_DurationOps::create(duration);
                auto res = self->get_ticks(out_ticks);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ratio_num =
                [](const TF_Duration* duration, int64_t* out_num) noexcept
            {
                auto* self = TF_DurationOps::create(duration);
                auto res = self->get_ratio_num(out_num);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ratio_den =
                [](const TF_Duration* duration, int64_t* out_den) noexcept
            {
                auto* self = TF_DurationOps::create(duration);
                auto res = self->get_ratio_den(out_den);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_DurationOps::create(plugin_context);
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
