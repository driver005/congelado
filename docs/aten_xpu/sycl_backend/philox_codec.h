// Portable Philox counter math — unchanged from core/PhiloxXpuState.h, just retargeted at the C
// ABI's TF_PhiloxState instead of at::PhiloxXpuState. Header-only, included (not imported) since
// it is a small leaf utility a kernel translation unit pulls in directly.
#pragma once

#include "include/c/extern/random_generator/random_generator.h"

#include <cstdint>
#include <utility>

namespace sycl_backend {

// Turns a TF_PhiloxState (filled by SyclRandomGenerator::philox_state) into the (seed, offset)
// pair a kernel's Philox counter actually needs, whether the generator was captured into a
// device graph or not.
class PhiloxCodec
{
public:
    PhiloxCodec() = delete;

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

} // namespace sycl_backend
