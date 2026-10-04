module;

#include <sycl/sycl.hpp>

export module aten_xpu_extern_random_generator:philox_state;

import std;

export namespace aten_xpu {

class SyclPhiloxState
{
public:
    static constexpr uint64_t k_round_size = 4;
    static constexpr uint64_t k_default_seed = 67280421310721ULL;

    explicit SyclPhiloxState(uint64_t seed, uint64_t offset = 0, uint32_t offset_intragraph = 0) noexcept :
        m_seed{seed},
        m_offset{offset},
        m_offset_intragraph{offset_intragraph}
    {
    }

    ~SyclPhiloxState() { free_extragraph(); }

    SyclPhiloxState(const SyclPhiloxState&) = delete;
    SyclPhiloxState& operator=(const SyclPhiloxState&) = delete;
    SyclPhiloxState(SyclPhiloxState&&) = delete;
    SyclPhiloxState& operator=(SyclPhiloxState&&) = delete;

    void setSeed(uint64_t seed) noexcept
    {

        m_seed = seed;
        m_offset = 0;

    }

    void setOffset(uint64_t offset) noexcept
    {

        if (m_capturing) {
            m_offset_intragraph = static_cast<uint32_t>(offset);
        } else {
            m_offset = offset;
        }

    }

    uint64_t getSeed() const noexcept { return m_seed; }

    uint64_t getOffset() const noexcept
    {

        return m_capturing ? m_offset_intragraph : m_offset;

    }

    uint32_t getOffsetIntragraph() const noexcept { return m_offset_intragraph; }

    bool getCapturing() const noexcept { return m_capturing; }

    const int64_t* getSeedExtragraph() const noexcept { return m_seed_extragraph; }

    const int64_t* getOffsetExtragraph() const noexcept { return m_offset_extragraph; }

    std::shared_ptr<SyclPhiloxState> clone() const
    {

        return std::make_shared<SyclPhiloxState>(m_seed, m_offset, m_offset_intragraph);

    }

    void increase(uint64_t increment)
    {

        const auto rounded = (increment + k_round_size - 1) / k_round_size * k_round_size;
        if (m_capturing) {
            if (m_offset_intragraph > std::numeric_limits<uint32_t>::max() - rounded) {
                throw std::overflow_error{"philox intragraph offset overflow"};
            }
            m_offset_intragraph += static_cast<uint32_t>(rounded);
            return;
        }
        m_offset += rounded;

    }

    void register_graph(const void* graph)
    {

        if (m_capturing) {
            throw std::logic_error{"cannot register a graph while capturing"};
        }
        m_registered_graphs.insert(graph);

    }

    void unregister_graph(const void* graph)
    {

        if (m_registered_graphs.erase(graph) == 0) {
            throw std::logic_error{"graph is not registered with this generator"};
        }
        if (m_registered_graphs.empty()) {
            free_extragraph();
        }

    }

    void capture_prologue(sycl::queue& queue)
    {

        if (m_seed_extragraph == nullptr) {
            m_context = queue.get_context();
            m_seed_extragraph = sycl::malloc_device<int64_t>(1, queue);
            m_offset_extragraph = sycl::malloc_device<int64_t>(1, queue);
        }
        m_capturing = true;
        m_offset_intragraph = 0;
        queue.fill(m_seed_extragraph, static_cast<int64_t>(m_seed), 1);
        queue.fill(m_offset_extragraph, int64_t{0}, 1);

    }

    uint64_t capture_epilogue() noexcept
    {

        m_capturing = false;
        return m_offset_intragraph;

    }

    void replay_prologue(sycl::queue& queue, uint64_t whole_graph_increment)
    {

        if (whole_graph_increment == 0) {
            return;
        }
        queue.fill(m_seed_extragraph, static_cast<int64_t>(m_seed), 1);
        queue.fill(m_offset_extragraph, static_cast<int64_t>(m_offset), 1);
        increase(whole_graph_increment);

    }

private:
    void free_extragraph() noexcept
    {

        if (m_context) {
            sycl::free(m_seed_extragraph, *m_context);
            sycl::free(m_offset_extragraph, *m_context);
        }
        m_seed_extragraph = nullptr;
        m_offset_extragraph = nullptr;

    }

    uint64_t m_seed;
    uint64_t m_offset;
    uint32_t m_offset_intragraph;
    bool m_capturing{false};
    std::set<const void*> m_registered_graphs;
    std::optional<sycl::context> m_context;
    int64_t* m_seed_extragraph{nullptr};
    int64_t* m_offset_extragraph{nullptr};
};

} // namespace aten_xpu
