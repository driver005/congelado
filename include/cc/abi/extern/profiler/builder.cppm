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
        return reinterpret_cast<Profiler*>(handle);
    }

    virtual ~Profiler() = default;
    [[nodiscard]] std::expected<void, ice::Status> get_device_type(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> start() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> collect_data_xspace() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Profiler* get_generic_vtable()
    {
        static TF_Profiler vtable = {
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
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->get_device_type(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stop =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .collect_data_xspace =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->collect_data_xspace();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
