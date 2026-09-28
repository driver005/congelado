// SYCL reference plugin — Philox RNG state, one per TF_RandomGenerator.
//
// Not built by Bazel (docs/ only). Replaces core/XPUGeneratorImpl.{h,cpp}. The device-side
// extragraph seed/offset tensors that graph-safe mode needs (XPUGeneratorState::seed_extragraph_
// /offset_extragraph_) depend on SyclTensor, which is a later step of this conversion; until
// then graphsafe_set_state/get_state and set_state/get_state return "not implemented" rather
// than silently doing the non-graph-safe thing.

module;

#include "include/c/extern/random_generator/random_generator.h"

export module sycl_backend:random_generator;

import std;
import cc_ice_extern_random_generator_builder;

export namespace sycl_backend {

class SyclRandomGenerator : public ice::builder::RandomGenerator
{
public:
    // Philox advances in blocks of 4; matches core/XPUGeneratorImpl.cpp's PHILOX_ROUND_SIZE.
    static constexpr uint64_t PHILOX_ROUND_SIZE = 4;

    SyclRandomGenerator(int device_index, uint64_t seed) noexcept :
        m_device_index{device_index},
        m_seed{seed}
    {
    }

    ~SyclRandomGenerator() override = default;
    SyclRandomGenerator(const SyclRandomGenerator&) = delete;
    SyclRandomGenerator& operator=(const SyclRandomGenerator&) = delete;
    SyclRandomGenerator(SyclRandomGenerator&&) = delete;
    SyclRandomGenerator& operator=(SyclRandomGenerator&&) = delete;

    void set_seed(uint64_t seed) noexcept override
    {
        m_seed = seed;
        m_philox_offset = 0;
    }

    void get_seed(uint64_t* out_seed) noexcept override
    {
        *out_seed = m_seed;
    }

    void reseed_nondeterministic(uint64_t* out_seed) noexcept override
    {
        std::random_device entropy_source;
        std::uniform_int_distribution<uint64_t> distribution;
        m_seed = distribution(entropy_source);
        m_philox_offset = 0;
        *out_seed = m_seed;
    }

    void set_offset(uint64_t offset) noexcept override
    {
        m_philox_offset = offset;
    }

    void get_offset(uint64_t* out_offset) noexcept override
    {
        *out_offset = m_philox_offset;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_state(ice::builder::Tensor& state) noexcept
    {
        (void)state;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclRandomGenerator: set_state needs SyclTensor")
        };
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_state(TF_Tensor** out_state) noexcept override
    {
        (void)out_state;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclRandomGenerator: get_state needs SyclTensor")
        };
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    graphsafe_set_state(ice::builder::RandomGenerator& other) noexcept
    {
        (void)other;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclRandomGenerator: graph-safe state needs SyclTensor")
        };
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    graphsafe_get_state(TF_RandomGenerator* out_other) noexcept
    {
        (void)out_other;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclRandomGenerator: graph-safe state needs SyclTensor")
        };
    }

    // Advances the offset by increment (rounded up to a Philox block) and reports the (seed,
    // offset) pair the caller should have used, matching XPUGeneratorState::increase +
    // philox_engine_inputs. Not graph-safe (m_registered_graphs empty) — see the file-level note.
    [[nodiscard]] std::expected<void, ice::sonic::Status>
    philox_state(uint64_t increment, TF_PhiloxState* out_state) noexcept override
    {
        if (!m_registered_graphs.empty()) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclRandomGenerator: capturing generator needs graph-safe state, not implemented")
            };
        }

        const uint64_t rounded = round_up_to_block(increment);

        *out_state = TF_PhiloxState{
            .struct_size = TF_PHILOX_STATE_STRUCT_SIZE,
            .seed = m_seed,
            .offset = m_philox_offset,
            .seed_ptr = nullptr,
            .offset_ptr = nullptr,
            .offset_intragraph = 0,
            .captured = false
        };

        m_philox_offset += rounded;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    philox_engine_inputs(uint64_t increment, uint64_t* out_seed, uint64_t* out_offset) noexcept
        override
    {
        *out_seed = m_seed;
        *out_offset = m_philox_offset;
        m_philox_offset += round_up_to_block(increment);
        return {};
    }

    void get_device_index(int* out_device_index) noexcept override
    {
        *out_device_index = m_device_index;
    }

    // TF_RandomGeneratorOps has no destroy slot of its own — only
    // TF_ExecutorOps::destroy_random_generator_internal frees one, and only for generators its
    // own m_random_generators tracks (executor.cppm). A clone here is not registered anywhere,
    // so destroying it is currently the caller's responsibility via `delete`, not through the
    // executor; a real implementation would take the owning SyclExecutor as a constructor
    // argument so clone() could register itself the same way create_random_generator_internal does.
    [[nodiscard]] std::expected<void, ice::sonic::Status>
    clone(TF_RandomGenerator* out_clone) noexcept override
    {
        auto* owned = new SyclRandomGenerator(m_device_index, m_seed);
        owned->m_philox_offset = m_philox_offset;
        out_clone->plugin_data = owned;
        return {};
    }

    void register_graph(void* graph) noexcept
    {
        m_registered_graphs.insert(graph);
    }

    void unregister_graph(void* graph) noexcept
    {
        m_registered_graphs.erase(graph);
    }

private:
    static uint64_t round_up_to_block(uint64_t increment) noexcept
    {
        return ((increment + PHILOX_ROUND_SIZE - 1) / PHILOX_ROUND_SIZE) * PHILOX_ROUND_SIZE;
    }

    int m_device_index;
    uint64_t m_seed;
    uint64_t m_philox_offset{0};
    std::set<void*> m_registered_graphs;
};

} // namespace sycl_backend
