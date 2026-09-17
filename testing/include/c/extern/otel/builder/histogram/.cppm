// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/histogram.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/histogram.h"

export module cc_abi_builder_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFOtelHistogramOps
{
public:
    static TFOtelHistogramOps* create(void* ctx) noexcept
    {
        return static_cast<TFOtelHistogramOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelHistogramOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFOtelHistogramOps*>(handle->plugin_data);
    }

    virtual ~TFOtelHistogramOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> record(double value) noexcept = 0;

    static TFOtelHistogramOps* get_generic_vtable()
    {
        static TFOtelHistogramOps vtable = {
            .struct_size = TF_TELHISTOGRAM_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFOtelHistogramOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFOtelHistogramOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .record =
                [](TFOtelHistogram* histogram, double value, TF_Status* out_status) noexcept
            {
                auto* self = TFOtelHistogramOps::create(histogram);
                auto res = self->record(value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
