// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/counter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/counter.h"

export module cc_ice_builder_otel:counter;

import std;

export namespace ice::builder {

class TFOtelCounterOps
{
public:
    static TFOtelCounterOps* create(void* ctx) noexcept
    {
        return static_cast<TFOtelCounterOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelCounterOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFOtelCounterOps*>(handle->plugin_data);
    }

    virtual ~TFOtelCounterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> add(double value) noexcept = 0;

    static TFOtelCounterOps* get_generic_vtable()
    {
        static TFOtelCounterOps vtable = {
            .struct_size = TF_TELCOUNTER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFOtelCounterOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFOtelCounterOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .add =
                [](TFOtelCounter* counter, double value, TF_Status* out_status) noexcept
            {
                auto* self = TFOtelCounterOps::create(counter);
                auto res = self->add(value);
                if (!res) {
                    res.error().to_c(out_status);
                }
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
