module;

#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/types.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_random_generator:random_generator;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_random_generator_builder;
import aten_xpu_intern;
import :philox_state;

export namespace aten_xpu {

class SyclRandomGenerator : public ice::builder::TF_RandomGeneratorOps
{
public:
    static constexpr std::size_t k_state_bytes = 2 * sizeof(uint64_t);

    explicit SyclRandomGenerator(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_RandomGeneratorOps{ops.getRandomGeneratorOps(), ops.getStatusOps(), ops.getTensorOps()},
        m_status{ops}
    {
    }

    ~SyclRandomGenerator() override = default;
    SyclRandomGenerator(const SyclRandomGenerator&) = delete;
    SyclRandomGenerator& operator=(const SyclRandomGenerator&) = delete;
    SyclRandomGenerator(SyclRandomGenerator&&) = delete;
    SyclRandomGenerator& operator=(SyclRandomGenerator&&) = delete;

    static void create(::TF_RandomGenerator* handle)
    {

        auto* generator = new SyclRandomGenerator{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *generator);

    }

    void bind(std::shared_ptr<SyclPhiloxState> state, int device_index) noexcept
    {

        m_state = std::move(state);
        m_device_index = device_index;

    }

    void release() noexcept { m_state.reset(); }

    void destroy() noexcept override { delete this; }

    void set_seed(uint64_t seed) noexcept override
    {

        if (!m_state->getCapturing()) {
            m_state->setSeed(seed);
        }

    }

    void get_seed(uint64_t* out_seed) noexcept override { *out_seed = m_state->getSeed(); }

    void reseed_nondeterministic(uint64_t* out_seed) noexcept override
    {

        std::random_device entropy;
        const auto seed = (static_cast<uint64_t>(entropy()) << 32U) | entropy();
        m_state->setSeed(seed);
        *out_seed = seed;

    }

    void set_offset(uint64_t offset) noexcept override
    {

        if (offset % SyclPhiloxState::k_round_size == 0) {
            m_state->setOffset(offset);
        }

    }

    void get_offset(uint64_t* out_offset) noexcept override { *out_offset = m_state->getOffset(); }

    void set_state(const ice::sonic::TF_TensorOps& state, const ice::sonic::Status& out_status) noexcept override
    {

        auto& tensor = SyclHandle::resolve<SyclTensor>(state);
        const auto bytes = tensor.getByteSize();
        if (bytes != k_state_bytes && bytes != sizeof(uint64_t)) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "RNG state is wrong size");
            return;
        }

        std::array<uint64_t, 2> values{};
        try {
            read_host(tensor, values.data(), bytes);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
            return;
        }

        if (m_state->getCapturing() && values[0] != m_state->getSeed()) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "cannot change seed while capturing");
            return;
        }
        m_state->setSeed(values[0]);
        m_state->setOffset(bytes == k_state_bytes ? values[1] : 0);

    }

    void get_state(TF_Tensor** out_state, const ice::sonic::Status& out_status) noexcept override
    {

        try {
            const std::array<int64_t, 1> dims{static_cast<int64_t>(k_state_bytes)};
            auto* handle = SyclTensorFactory::empty(dims, TF_UINT8, m_device_index, TF_MEMORY_SPACE_HOST_PINNED);
            const std::array<uint64_t, 2> values{m_state->getSeed(), m_state->getOffset()};
            std::memcpy(SyclHandle::resolve_raw<SyclTensor>(handle).getData(), values.data(), k_state_bytes);
            *out_state = handle;
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void graphsafe_set_state(const ice::sonic::TF_RandomGeneratorOps& other, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(out_status);
        m_state = SyclHandle::resolve<SyclRandomGenerator>(other).m_state;

    }

    void graphsafe_get_state(
        const ice::sonic::TF_RandomGeneratorOps& out_other,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        SyclHandle::resolve<SyclRandomGenerator>(out_other).bind(m_state, m_device_index);

    }

    void philox_state(uint64_t increment, TF_PhiloxState* out_state, const ice::sonic::Status& out_status)
        noexcept override
    {

        try {
            *out_state = TF_PhiloxState{.struct_size = sizeof(TF_PhiloxState)};
            if (m_state->getCapturing()) {
                out_state->seed_ptr = m_state->getSeedExtragraph();
                out_state->offset_ptr = m_state->getOffsetExtragraph();
                out_state->offset_intragraph = m_state->getOffsetIntragraph();
                out_state->captured = true;
            } else {
                out_state->seed = m_state->getSeed();
                out_state->offset = m_state->getOffset();
            }
            m_state->increase(increment);
        } catch (const std::exception& error) {
            m_status.fail(out_status, TF_INTERNAL, error.what());
        }

    }

    void philox_engine_inputs(
        uint64_t increment,
        uint64_t* out_seed,
        uint64_t* out_offset,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (m_state->getCapturing()) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "use philox_state while capturing");
            return;
        }
        *out_seed = m_state->getSeed();
        *out_offset = m_state->getOffset();
        m_state->increase(increment);

    }

    void get_device_index(int* out_device_index) noexcept override { *out_device_index = m_device_index; }

    void clone(const ice::sonic::TF_RandomGeneratorOps& out_clone, const ice::sonic::Status& out_status)
        noexcept override
    {

        if (m_state->getCapturing()) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "cannot clone while capturing");
            return;
        }
        SyclHandle::resolve<SyclRandomGenerator>(out_clone).bind(m_state->clone(), m_device_index);

    }

    SyclPhiloxState& getState() const noexcept { return *m_state; }

    const std::shared_ptr<SyclPhiloxState>& getSharedState() const noexcept { return m_state; }

private:
    static void read_host(const SyclTensor& tensor, void* destination, std::size_t bytes)
    {

        if (tensor.getMemorySpace() != TF_MEMORY_SPACE_DEVICE) {
            std::memcpy(destination, tensor.getData(), bytes);
            return;
        }
        sycl::queue queue;
        queue.memcpy(destination, tensor.getData(), bytes).wait_and_throw();

    }

    SyclStatus m_status;
    std::shared_ptr<SyclPhiloxState> m_state;
    int m_device_index{0};
};

} // namespace aten_xpu
