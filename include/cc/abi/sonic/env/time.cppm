module;

#include <ctime>

export module cc_abi_sonic_env:time;

import std;

export namespace ice::sonic {

// Real host time source — include/c/ is declaration-only, so this doesn't forward to
// c/extern/env/env.h's now_* vtable slots (which have no implementation anywhere); it calls
// clock_gettime directly, same "real host implementation" choice as DynamicLibrary's
// dlopen/dlsym.
class TimeRuntime
{
public:
    static uint64_t now_nanos()
    {
        timespec ts{};
        clock_gettime(CLOCK_REALTIME, &ts);
        return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
               static_cast<uint64_t>(ts.tv_nsec);
    }

    static uint64_t now_micros()
    {
        return now_nanos() / 1'000;
    }

    static uint64_t now_seconds()
    {
        return now_nanos() / 1'000'000'000ULL;
    }
};

} // namespace ice::sonic
