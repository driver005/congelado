export module aten_xpu_profiler_common:kind;

import std;

export namespace aten_xpu {

// What a recorded piece of device or host work was.
enum class ProfileKind : std::uint8_t
{
    COMPUTE,
    COPY,
    HOST,
};

} // namespace aten_xpu
