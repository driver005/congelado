// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/timepoint/timepoint.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/timepoint/timepoint.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_timepoint;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Timepoint
{
public:
    static Timepoint* create(void* ctx) noexcept
    {
        return static_cast<Timepoint*>(ctx);
    }

    template<typename HandleT>
    static Timepoint* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Timepoint*>(handle);
    }

    virtual ~Timepoint() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_time_point(int64_t ticks, int64_t ratio_num, int64_t ratio_den) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_duration_since_epoch() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_TimePoint* get_generic_vtable()
    {
        static TF_TimePoint vtable = {
            .struct_size = TF_TIMEPOINT_STRUCT_SIZE,
            .new_time_point =
                [](void* plugin_context,
                   int64_t ticks,
                   int64_t ratio_num,
                   int64_t ratio_den) noexcept
            {
                auto* self = Timepoint::create(plugin_context);
                auto res = self->new_time_point(ticks, ratio_num, ratio_den);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_duration_since_epoch =
                [](const TF_TimePoint_Handle* time_point) noexcept
            {
                auto* self = Timepoint::create(time_point);
                auto res = self->get_duration_since_epoch();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Timepoint::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
