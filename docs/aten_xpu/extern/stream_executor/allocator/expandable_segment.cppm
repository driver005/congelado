module;

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_expandable_segment;

import std;

export namespace aten_xpu {

class SyclExpandableSegment
{
public:
    SyclExpandableSegment(
        const sycl::context& context,
        const sycl::device& device,
        std::size_t segment_size
    ) :
        m_context{context},
        m_device{device},
        m_segment_size{segment_size}
    {
        namespace experimental = sycl::ext::oneapi::experimental;

        const auto granularity = experimental::get_mem_granularity(
            m_device,
            m_context,
            experimental::granularity_mode::minimum
        );
        if (m_segment_size % granularity != 0) {
            throw std::invalid_argument{"segment size must be a multiple of the granularity"};
        }

        const auto device_total = m_device.get_info<sycl::info::device::global_mem_size>();
        m_max_handles = segment_count(device_total + device_total / 8);
        m_base = experimental::reserve_virtual_mem(m_segment_size * m_max_handles, m_context);
        if (m_base == 0 || m_base % granularity != 0) {
            throw std::runtime_error{"failed to reserve aligned virtual memory"};
        }
    }

    ~SyclExpandableSegment()
    {
        for (std::size_t index = 0; index < m_handles.size(); ++index) {
            if (m_handles[index]) {
                unmap_handle(index);
            }
        }
        sycl::ext::oneapi::experimental::free_virtual_mem(
            m_base,
            m_segment_size * m_max_handles,
            m_context
        );
    }

    SyclExpandableSegment(const SyclExpandableSegment&) = delete;
    SyclExpandableSegment& operator=(const SyclExpandableSegment&) = delete;
    SyclExpandableSegment(SyclExpandableSegment&&) = delete;
    SyclExpandableSegment& operator=(SyclExpandableSegment&&) = delete;

    std::byte* getPointer() const noexcept
    {
        return reinterpret_cast<std::byte*>(m_base);
    }

    std::size_t getSize() const noexcept
    {
        return m_max_handles * m_segment_size;
    }

    std::span<std::byte> map(std::span<std::byte> range)
    {
        namespace experimental = sycl::ext::oneapi::experimental;

        const auto begin = segment_left(range.data());
        const auto end = segment_right(range.data() + range.size());
        if (begin == end) {
            return range_of(begin, end);
        }
        if (end > m_handles.size()) {
            m_handles.resize(end);
        }

        for (auto index = begin; index < end; ++index) {
            try {
                auto& memory = m_handles[index].emplace(m_device, m_context, m_segment_size);
                memory.map(
                    m_base + index * m_segment_size,
                    m_segment_size,
                    experimental::address_access_mode::read_write
                );
            } catch (const sycl::exception&) {
                m_handles[index].reset();
                for (auto rollback = begin; rollback < index; ++rollback) {
                    unmap_handle(rollback);
                }
                trim_handles();
                return range_of(begin, begin);
            }
        }
        return range_of(begin, end);
    }

    std::span<std::byte> unmap(std::span<std::byte> range)
    {
        const auto begin = segment_right(range.data());
        const auto end = segment_left(range.data() + range.size());
        if (begin >= end) {
            return {range.data(), 0};
        }
        for (auto index = begin; index < end; ++index) {
            unmap_handle(index);
        }
        trim_handles();
        return range_of(begin, end);
    }

private:
    void unmap_handle(std::size_t index)
    {
        sycl::ext::oneapi::experimental::unmap(
            reinterpret_cast<void*>(m_base + m_segment_size * index),
            m_segment_size,
            m_context
        );
        m_handles[index].reset();
    }

    void trim_handles()
    {
        while (!m_handles.empty() && !m_handles.back()) {
            m_handles.pop_back();
        }
    }

    std::size_t segment_count(std::size_t size) const noexcept
    {
        return (size + m_segment_size - 1) / m_segment_size;
    }

    std::size_t segment_left(const std::byte* pointer) const noexcept
    {
        return static_cast<std::size_t>(pointer - getPointer()) / m_segment_size;
    }

    std::size_t segment_right(const std::byte* pointer) const noexcept
    {
        return segment_count(static_cast<std::size_t>(pointer - getPointer()));
    }

    std::span<std::byte> range_of(std::size_t begin, std::size_t end) const noexcept
    {
        return {getPointer() + m_segment_size * begin, m_segment_size * (end - begin)};
    }

    sycl::context m_context;
    sycl::device m_device;
    std::size_t m_segment_size;
    std::size_t m_max_handles{0};
    std::uintptr_t m_base{0};
    std::vector<std::optional<sycl::ext::oneapi::experimental::physical_mem>> m_handles;
};

} // namespace aten_xpu
