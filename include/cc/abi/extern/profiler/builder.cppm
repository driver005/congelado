// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/profiler/profiler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/profiler/profiler.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_profiler;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Profiler
{
public:
    static Profiler* create(void* ctx) noexcept
    {
        return static_cast<Profiler*>(ctx);
    }

    template<typename HandleT>
    static Profiler* create(HandleT* handle) noexcept
    {
        return static_cast<Profiler*>(handle->plugin_data);
    }

    virtual ~Profiler() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    get_device_type(const ice::sonic::String& out_device_type) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> start() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    collect_data_xspace(TF_Tensor** out_data) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_ProfilerOps* get_generic_vtable()
    {
        static TF_ProfilerOps vtable = {
            .struct_size = TF_PROFILER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Profiler::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_device_type =
                [](TF_Profiler* profiler, TF_String* out_device_type) noexcept
            {
                auto* self = Profiler::create(profiler);
                auto res = self->get_device_type(ice::sonic::String::wrap(out_device_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start =
                [](TF_Profiler* profiler, TF_Status* out_status) noexcept
            {
                auto* self = Profiler::create(profiler);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop =
                [](TF_Profiler* profiler, TF_Status* out_status) noexcept
            {
                auto* self = Profiler::create(profiler);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .collect_data_xspace =
                [](TF_Profiler* profiler, TF_Tensor** out_data, TF_Status* out_status) noexcept
            {
                auto* self = Profiler::create(profiler);
                auto res = self->collect_data_xspace(out_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
