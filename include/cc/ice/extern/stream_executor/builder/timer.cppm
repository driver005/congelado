// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/timer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/timer.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_stream_executor_builder:timer;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Timer
{
public:
    explicit Timer() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    static Timer& from_handle(void* ctx) noexcept
    {
        return *static_cast<Timer*>(ctx);
    }

    template<typename HandleT>
    static Timer& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Timer*>(handle->plugin_data);
    }

    virtual ~Timer() = default;
    virtual void destroy() noexcept = 0;
    virtual void nanoseconds(uint64_t* out_nanoseconds) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Timer*)) noexcept
    {
        m_vtable = ::TF_TimerOps{
            .struct_size = TF_OFFSET_OF_END(::TF_TimerOps, nanoseconds),

            .create = create,
            .destroy =
                [](::TF_Timer* handle) noexcept
            {
                auto& self = Timer::from_handle(handle);
                self.destroy();
            },
            .nanoseconds =
                [](::TF_Timer* timer, uint64_t* out_nanoseconds) noexcept
            {
                auto& self = Timer::from_handle(timer);
                self.nanoseconds(out_nanoseconds);
            },

        };
    }

    const ::TF_TimerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Timer& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_TimerOps*>(&m_vtable));
    }

private:
    ::TF_TimerOps m_vtable;
    ::TF_Timer m_handle;
};

} // namespace ice::builder
