module;

#include "include/c/extern/random_generator/random_generator.h"

export module aten_xpu_extern_random_generator:philox_codec;

import std;
import cc_ice_extern_random_generator_sonic;

export namespace aten_xpu {

class SyclPhiloxCodec
{
public:
    std::pair<uint64_t, uint64_t> unpack(const ice::sonic::PhiloxState& state) const noexcept
    {
        if (state.getCaptured()) {
            return {
                static_cast<uint64_t>(*state.getSeedPtr()),
                static_cast<uint64_t>(*state.getOffsetPtr()) + state.getOffsetIntragraph()
            };
        }
        return {state.getSeed(), state.getOffset()};
    }
};

} // namespace aten_xpu
