// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/timer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/timer.h"

export module cc_ice_extern_stream_executor_sonic:timer;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TimerOps : public ice::sonic::Runtime<TF_TimerOps, TF_TimerOps>
{
public:
    explicit TF_TimerOps(TF_TimerOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "stream_executor";

    void nanoseconds(uint64_t* out_nanoseconds) noexcept
    {
        m_ops->nanoseconds(get_handle(), out_nanoseconds);
    }
};

} // namespace ice::sonic
