module;

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_host_cache;

import std;

export namespace aten_xpu {

class SyclHostCache
{
public:
    static constexpr std::size_t k_host_alignment = 512;

    explicit SyclHostCache(const sycl::context& context) :
        m_context{context}
    {
    }

    ~SyclHostCache()
    {
        release_all();
    }

    SyclHostCache(const SyclHostCache&) = delete;
    SyclHostCache& operator=(const SyclHostCache&) = delete;
    SyclHostCache(SyclHostCache&&) = delete;
    SyclHostCache& operator=(SyclHostCache&&) = delete;

    void* allocate(std::size_t size)
    {
        process_events();

        const auto rounded = std::bit_ceil(std::max(size, k_host_alignment));
        auto& bucket = m_free[rounded];
        if (!bucket.empty()) {
            void* pointer = bucket.back();
            bucket.pop_back();
            m_sizes[pointer] = rounded;
            return pointer;
        }

        void* pointer = sycl::aligned_alloc_host(k_host_alignment, rounded, m_context);
        if (pointer == nullptr) {
            empty_cache();
            pointer = sycl::aligned_alloc_host(k_host_alignment, rounded, m_context);
        }
        if (pointer != nullptr) {
            m_sizes[pointer] = rounded;
            m_reserved_bytes += rounded;
        }
        return pointer;
    }

    void record_stream(void* pointer, sycl::queue& queue)
    {
        if (m_sizes.contains(pointer)) {
            m_pending_streams[pointer].insert(&queue);
        }
    }

    bool deallocate(void* pointer)
    {
        const auto found = m_sizes.find(pointer);
        if (found == m_sizes.end()) {
            return false;
        }

        auto streams = m_pending_streams.extract(pointer);
        if (streams.empty() || streams.mapped().empty()) {
            m_free[found->second].push_back(pointer);
            return true;
        }

        for (auto* queue: streams.mapped()) {
            m_events.emplace_back(queue->ext_oneapi_submit_barrier(), pointer);
        }
        m_outstanding[pointer] = static_cast<int>(streams.mapped().size());
        return true;
    }

    bool owns(const void* pointer) const
    {
        return m_sizes.contains(const_cast<void*>(pointer));
    }

    void empty_cache()
    {
        process_events();
        for (auto& [size, bucket]: m_free) {
            for (void* pointer: bucket) {
                sycl::free(pointer, m_context);
                m_sizes.erase(pointer);
                m_reserved_bytes -= size;
            }
            bucket.clear();
        }
    }

    std::size_t getReservedBytes() const noexcept
    {
        return m_reserved_bytes;
    }

private:
    void process_events()
    {
        std::erase_if(
            m_events,
            [this](const std::pair<sycl::event, void*>& entry)
            {
                if (entry.first.get_info<sycl::info::event::command_execution_status>() !=
                    sycl::info::event_command_status::complete) {
                    return false;
                }
                if (--m_outstanding[entry.second] == 0) {
                    m_outstanding.erase(entry.second);
                    m_free[m_sizes.at(entry.second)].push_back(entry.second);
                }
                return true;
            }
        );
    }

    void release_all() noexcept
    {
        for (const auto& [pointer, size]: m_sizes) {
            sycl::free(pointer, m_context);
        }
        m_sizes.clear();
        m_free.clear();
    }

    sycl::context m_context;
    std::map<std::size_t, std::vector<void*>> m_free;
    std::unordered_map<void*, std::size_t> m_sizes;
    std::unordered_map<void*, std::set<sycl::queue*>> m_pending_streams;
    std::unordered_map<void*, int> m_outstanding;
    std::vector<std::pair<sycl::event, void*>> m_events;
    std::size_t m_reserved_bytes{0};
};

} // namespace aten_xpu
