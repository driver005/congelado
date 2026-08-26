module;

#include "c/extern/env/time.h"

export module cc_abi_sonic_env:time;

export namespace ice::sonic {

class TimeRuntime
{
public:
    static uint64_t now_nanos()
    {
        return TF_NowNanos();
    }

    static uint64_t now_micros()
    {
        return TF_NowMicros();
    }

    static uint64_t now_seconds()
    {
        return TF_NowSeconds();
    }
};

} // namespace ice::sonic
