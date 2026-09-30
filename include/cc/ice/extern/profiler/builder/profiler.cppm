// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/profiler/profiler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/profiler/profiler.h"

export module cc_ice_extern_profiler_builder:profiler;

import std;

export namespace ice::builder {

class TF_ProfilerOps
{
public:
    TF_ProfilerOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ProfilerOps(const TF_ProfilerOps&) = delete;
    TF_ProfilerOps& operator=(const TF_ProfilerOps&) = delete;

    static TF_ProfilerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ProfilerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ProfilerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ProfilerOps*>(handle->plugin_data);
    }

    virtual ~TF_ProfilerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_type(const ice::sonic::String& out_device_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> start() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stop() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    collect_data_xspace(TF_Tensor** out_data) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ProfilerOps{
            .struct_size = TF_PROFILER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_ProfilerOps>{&TF_ProfilerOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_ProfilerOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .get_device_type =
                [](TF_Profiler* profiler, TF_String* out_device_type) noexcept
            {
                auto res = TF_ProfilerOps::from_handle(profiler).get_device_type(
                    ice::sonic::String::wrap(out_device_type)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start =
                [](TF_Profiler* profiler, TF_Status* out_status) noexcept
            {
                auto res = TF_ProfilerOps::from_handle(profiler).start();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop =
                [](TF_Profiler* profiler, TF_Status* out_status) noexcept
            {
                auto res = TF_ProfilerOps::from_handle(profiler).stop();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .collect_data_xspace =
                [](TF_Profiler* profiler, TF_Tensor** out_data, TF_Status* out_status) noexcept
            {
                auto res = TF_ProfilerOps::from_handle(profiler).collect_data_xspace(out_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_ProfilerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Profiler& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ProfilerOps m_vtable;
    TF_Profiler m_handle;
};

} // namespace ice::builder
