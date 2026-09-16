// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/duration/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/duration/duration.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_duration;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Duration
{
public:
    static Duration* create(void* ctx) noexcept
    {
        return static_cast<Duration*>(ctx);
    }

    template<typename HandleT>
    static Duration* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Duration*>(handle);
    }

    virtual ~Duration() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_duration(int64_t ticks, int64_t ratio_num, int64_t ratio_den) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_ticks() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_ratio_num() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_ratio_den() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Duration* get_generic_vtable()
    {
        static TF_Duration vtable = {
            .struct_size = TF_DURATION_STRUCT_SIZE,
            .new_duration =
                [](void* plugin_context,
                   int64_t ticks,
                   int64_t ratio_num,
                   int64_t ratio_den) noexcept
            {
                auto* self = Duration::create(plugin_context);
                auto res = self->new_duration(ticks, ratio_num, ratio_den);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ticks =
                [](const TF_Duration_Handle* duration) noexcept
            {
                auto* self = Duration::create(duration);
                auto res = self->get_ticks();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ratio_num =
                [](const TF_Duration_Handle* duration) noexcept
            {
                auto* self = Duration::create(duration);
                auto res = self->get_ratio_num();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ratio_den =
                [](const TF_Duration_Handle* duration) noexcept
            {
                auto* self = Duration::create(duration);
                auto res = self->get_ratio_den();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Duration::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
