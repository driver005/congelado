module;

#include <cstdint>
#include <ctime>

export module cc_abi_sonic_env:time;

import std;

export namespace ice::sonic {

// Real host time source — include/c/ is declaration-only, and the c/extern/env/env.h
// vtable (with its now_* slots) was dead code and has been removed; this calls
// clock_gettime directly, same "real host implementation" choice as DynamicLibrary's
// dlopen/dlsym.
class Time
{
public:
    static uint64_t now_nanos() noexcept
    {
        timespec ts{};
        clock_gettime(CLOCK_REALTIME, &ts);
        return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
               static_cast<uint64_t>(ts.tv_nsec);
    }

    static uint64_t now_micros() noexcept
    {
        return now_nanos() / 1'000;
    }

    static uint64_t now_seconds() noexcept
    {
        return now_nanos() / 1'000'000'000ULL;
    }
};

} // namespace ice::sonic
