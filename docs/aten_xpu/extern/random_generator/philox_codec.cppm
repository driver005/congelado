module;

#include "include/c/extern/random_generator/random_generator.h"

export module aten_xpu_extern_random_generator:philox_codec;

import std;

export namespace aten_xpu {

class SyclPhiloxCodec
{
public:
    SyclPhiloxCodec() = delete;

    static std::pair<uint64_t, uint64_t> unpack(const TF_PhiloxState& state) noexcept
    {
        if (state.captured) {
            return {
                static_cast<uint64_t>(*state.seed_ptr),
                static_cast<uint64_t>(*state.offset_ptr) + state.offset_intragraph
            };
        }
        return {state.seed, state.offset};
    }
};

} // namespace aten_xpu
