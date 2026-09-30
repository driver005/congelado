// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/histogram.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/histogram.h"

export module cc_ice_extern_otel_builder:histogram;

import std;

export namespace ice::builder {

class TFOtelHistogramOps
{
public:
    TFOtelHistogramOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFOtelHistogramOps(const TFOtelHistogramOps&) = delete;
    TFOtelHistogramOps& operator=(const TFOtelHistogramOps&) = delete;

    static TFOtelHistogramOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFOtelHistogramOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelHistogramOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFOtelHistogramOps*>(handle->plugin_data);
    }

    virtual ~TFOtelHistogramOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> record(double value) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFOtelHistogramOps{
            .struct_size = TF_TELHISTOGRAM_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFOtelHistogramOps>{
                    &TFOtelHistogramOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFOtelHistogramOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .record =
                [](TFOtelHistogram* histogram, double value, TF_Status* out_status) noexcept
            {
                auto res = TFOtelHistogramOps::from_handle(histogram).record(value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFOtelHistogramOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFOtelHistogram& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFOtelHistogramOps m_vtable;
    TFOtelHistogram m_handle;
};

} // namespace ice::builder
