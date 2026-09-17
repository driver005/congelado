// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/time_point.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/time_point.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_TimePointOps
{
public:
    static TF_TimePointOps* create(void* ctx) noexcept
    {
        return static_cast<TF_TimePointOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TimePointOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_TimePointOps*>(handle->plugin_data);
    }

    virtual ~TF_TimePointOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    get_duration_since_epoch(const ice::sonic::TF_DurationOps& out_duration) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_TimePointOps* get_generic_vtable()
    {
        static TF_TimePointOps vtable = {
            .struct_size = TF_TIMEPOINT_STRUCT_SIZE,
            .get_duration_since_epoch =
                [](const TF_TimePoint* time_point, TF_Duration* out_duration) noexcept
            {
                auto* self = TF_TimePointOps::create(time_point);
                auto res =
                    self->get_duration_since_epoch(ice::sonic::TF_DurationOps::wrap(out_duration));
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_TimePointOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
