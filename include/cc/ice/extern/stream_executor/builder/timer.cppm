// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/timer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/timer.h"

export module cc_ice_extern_stream_executor_builder:timer;

import std;

export namespace ice::builder {

class TF_TimerOps
{
public:
    TF_TimerOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_TimerOps(const TF_TimerOps&) = delete;
    TF_TimerOps& operator=(const TF_TimerOps&) = delete;

    static TF_TimerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_TimerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TimerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_TimerOps*>(handle->plugin_data);
    }

    virtual ~TF_TimerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    nanoseconds(uint64_t* out_nanoseconds) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_TimerOps{
            .struct_size = TF_TIMER_STRUCT_SIZE,
            .nanoseconds = [](TF_Timer* timer, uint64_t* out_nanoseconds) noexcept
            {
                auto res = TF_TimerOps::from_handle(timer).nanoseconds(out_nanoseconds);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_TimerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Timer& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_TimerOps m_vtable;
    TF_Timer m_handle;
};

} // namespace ice::builder
