// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/timer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/timer.h"

export module cc_ice_builder_stream_executor:timer;

import std;

export namespace ice::builder {

class TF_TimerOps
{
public:
    static TF_TimerOps* create(void* ctx) noexcept
    {
        return static_cast<TF_TimerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TimerOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_TimerOps*>(handle->plugin_data);
    }

    virtual ~TF_TimerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    nanoseconds(uint64_t* out_nanoseconds) noexcept = 0;

    static TF_TimerOps* get_generic_vtable()
    {
        static TF_TimerOps vtable = {
            .struct_size = TF_TIMER_STRUCT_SIZE,
            .nanoseconds = [](TF_Timer* timer, uint64_t* out_nanoseconds) noexcept
            {
                auto* self = TF_TimerOps::create(timer);
                auto res = self->nanoseconds(out_nanoseconds);
                if (!res) {
                    res.error().to_c(status);
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
