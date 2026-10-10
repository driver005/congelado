// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/timer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/timer.h"

export module cc_ice_extern_stream_executor_builder:timer;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_TimerOps
{
public:
    explicit TF_TimerOps() noexcept :
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
    virtual void destroy() noexcept = 0;
    virtual void nanoseconds(uint64_t* out_nanoseconds) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Timer*)) noexcept
    {
        m_vtable = ::TF_TimerOps{
            .struct_size = TF_OFFSET_OF_END(::TF_TimerOps, nanoseconds),

            .create = create,
            .destroy =
                [](TF_Timer* handle) noexcept
            {
                auto& self = TF_TimerOps::from_handle(handle);
                self.destroy();
            },
            .nanoseconds =
                [](TF_Timer* timer, uint64_t* out_nanoseconds) noexcept
            {
                auto& self = TF_TimerOps::from_handle(timer);
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
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_TimerOps*>(&m_vtable)
        );
    }

private:
    ::TF_TimerOps m_vtable;
    ::TF_Timer m_handle;
};

} // namespace ice::builder
