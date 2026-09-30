// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/counter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/counter.h"

export module cc_ice_extern_otel_builder:counter;

import std;

export namespace ice::builder {

class TFOtelCounterOps
{
public:
    TFOtelCounterOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFOtelCounterOps(const TFOtelCounterOps&) = delete;
    TFOtelCounterOps& operator=(const TFOtelCounterOps&) = delete;

    static TFOtelCounterOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFOtelCounterOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelCounterOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFOtelCounterOps*>(handle->plugin_data);
    }

    virtual ~TFOtelCounterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> add(double value) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFOtelCounterOps{
            .struct_size = TF_TELCOUNTER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFOtelCounterOps>{&TFOtelCounterOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFOtelCounterOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .add =
                [](TFOtelCounter* counter, double value, TF_Status* out_status) noexcept
            {
                auto res = TFOtelCounterOps::from_handle(counter).add(value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFOtelCounterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFOtelCounter& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFOtelCounterOps m_vtable;
    TFOtelCounter m_handle;
};

} // namespace ice::builder
