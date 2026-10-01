// SYCL reference plugin — best-fit caching block allocator.
//
// Not built by Bazel (docs/ only). Replaces c10/XPUCachingAllocator.cpp (block/BlockPool,
// kDeviceAlignment=512) and core/CachingHostAllocator.cpp (TF_MEMORY_SPACE_HOST_PINNED here).
// One SyclAllocator per device, created by SyclMemory::create_allocator_internal. Simplified
// relative to the real allocator: no expandable segments, no IPC. Splitting/coalescing, the
// small/large size classes, stream-ordered deferred free via record_stream, memory-fraction
// limits and stats are real.

module;

#include "include/c/extern/stream_executor/allocator.h"

export module sycl_backend:allocator;

import std;
import cc_ice_extern_memory_builder;
import :device;
import :stream;
import :mem_pool;

export namespace sycl_backend {

class SyclAllocator : public ice::builder::Allocator
{
public:
    // Matches c10/XPUCachingAllocator.cpp's kDeviceAlignment / kSmallSize.
    static constexpr uint64_t DEVICE_ALIGNMENT = 512;
    static constexpr uint64_t SMALL_SIZE = 1U << 20;

    SyclAllocator(sycl::context& context, SyclDevice& device) noexcept :
        m_context{context},
        m_device{device}
    {
    }

    ~SyclAllocator() override
    {
        for (Segment& segment: m_segments) {
            sycl::free(segment.base, m_context);
        }
    }

    SyclAllocator(const SyclAllocator&) = delete;
    SyclAllocator& operator=(const SyclAllocator&) = delete;
    SyclAllocator(SyclAllocator&&) = delete;
    SyclAllocator& operator=(SyclAllocator&&) = delete;

    [[nodiscard]] std::expected<void, ice::sonic::Status> allocate(
        uint64_t size,
        TF_MemorySpace memory_space,
        ice::builder::Stream& stream,
        TF_DeviceMemoryBase* out_memory
    ) noexcept override
    {
        if (memory_space != TF_MEMORY_SPACE_DEVICE) {
            return allocate_unpooled(size, memory_space, out_memory);
        }

        const uint64_t rounded_size = round_size(size);
        Block* block = find_free_block(rounded_size, &native_queue(stream));

        if (block == nullptr) {
            auto grown = grow_segment(rounded_size);
            if (!grown) {
                return std::unexpected{grown.error()};
            }
            block = find_free_block(rounded_size, &native_queue(stream));
        }

        if (block == nullptr) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclAllocator: out of pool memory")
            };
        }

        split_if_worthwhile(*block, rounded_size);
        block->allocated = true;
        block->requested_size = size;

        *out_memory = TF_DeviceMemoryBase{
            .struct_size = TF_OFFSET_OF_END(TF_DeviceMemoryBase, payload),
            .ext = nullptr,
            .opaque = block->ptr,
            .size = size,
            .payload = std::bit_cast<uint64_t>(block)
        };

        m_stats.num_allocs += 1;
        m_stats.bytes_in_use += static_cast<int64_t>(block->size);
        m_stats.peak_bytes_in_use = std::max(m_stats.peak_bytes_in_use, m_stats.bytes_in_use);
        return {};
    }

    void deallocate(TF_DeviceMemoryBase* memory) noexcept override
    {
        if (memory == nullptr || memory->opaque == nullptr) {
            return;
        }

        if (memory->payload == 0) {
            // Came from allocate_unpooled (host/unified): free directly, no block bookkeeping.
            sycl::free(memory->opaque, m_context);
            memory->opaque = nullptr;
            return;
        }

        auto* block = std::bit_cast<Block*>(memory->payload);

        if (!block->stream_uses.empty()) {
            // Stream-ordered free: wait for every recording stream before the block is reusable,
            // matching XPUCachingAllocator's deferred free via outstanding events.
            for (sycl::queue* queue: block->stream_uses) {
                queue->wait();
            }
            block->stream_uses.clear();
        }

        m_stats.bytes_in_use -= static_cast<int64_t>(block->size);
        block->allocated = false;
        coalesce(*block);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    record_stream(const TF_DeviceMemoryBase* memory, ice::builder::Stream& stream) noexcept override
    {
        if (memory == nullptr || memory->payload == 0) {
            return {};
        }

        auto* block = std::bit_cast<Block*>(memory->payload);
        block->stream_uses.insert(&native_queue(stream));
        return {};
    }

    void owns_pointer(const void* pointer, bool* out_owns) noexcept override
    {
        *out_owns = find_segment_containing(pointer) != nullptr;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_base_allocation(const void* pointer, void** out_base, uint64_t* out_size) noexcept override
    {
        auto* block = find_block_containing(pointer);
        if (block == nullptr) {
            return std::unexpected{ice::sonic::Status::from_message(
                "SyclAllocator: pointer not owned by this allocator"
            )};
        }

        *out_base = block->ptr;
        *out_size = block->size;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> empty_cache() noexcept override
    {
        auto is_segment_free = [](const Segment& segment)
        {
            return std::ranges::all_of(
                segment.blocks,
                [](const std::unique_ptr<Block>& block)
                {
                    return !block->allocated;
                }
            );
        };

        std::erase_if(
            m_segments,
            [&](Segment& segment)
            {
                if (!is_segment_free(segment)) {
                    return false;
                }
                for (const std::unique_ptr<Block>& block: segment.blocks) {
                    m_free_blocks.erase(block.get());
                }
                sycl::free(segment.base, m_context);
                return true;
            }
        );

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_memory_fraction(double fraction) noexcept override
    {
        if (fraction < 0.0 || fraction > 1.0) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclAllocator: memory fraction out of [0,1]")
            };
        }

        m_memory_fraction = fraction;
        return {};
    }

    void get_memory_fraction(double* out_fraction) noexcept override
    {
        *out_fraction = m_memory_fraction;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_option(TF_AllocatorOption option, int64_t value) noexcept override
    {
        switch (option) {
            case TF_ALLOCATOR_OPTION_NO_SPLIT:
                m_no_split = value != 0;
                return {};
            case TF_ALLOCATOR_OPTION_USE_ON_OOM:
                m_use_on_oom = value != 0;
                return {};
            case TF_ALLOCATOR_OPTION_ROUND_UP_POWER2_DIVISIONS:
                m_round_up_power2_divisions = value;
                return {};
            case TF_ALLOCATOR_OPTION_MAX_SPLIT_SIZE:
                m_max_split_size = value;
                return {};
            case TF_ALLOCATOR_OPTION_GARBAGE_COLLECTION_THRESHOLD:
                m_garbage_collection_threshold = value;
                return {};
            case TF_ALLOCATOR_OPTION_EXPANDABLE_SEGMENTS:
                return std::unexpected{ice::sonic::Status::from_message(
                    "SyclAllocator: expandable segments not implemented"
                )};
        }

        return std::unexpected{ice::sonic::Status::from_message("SyclAllocator: unknown option")};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_option(TF_AllocatorOption option, int64_t* out_value) noexcept override
    {
        switch (option) {
            case TF_ALLOCATOR_OPTION_NO_SPLIT:
                *out_value = m_no_split ? 1 : 0;
                return {};
            case TF_ALLOCATOR_OPTION_USE_ON_OOM:
                *out_value = m_use_on_oom ? 1 : 0;
                return {};
            case TF_ALLOCATOR_OPTION_ROUND_UP_POWER2_DIVISIONS:
                *out_value = m_round_up_power2_divisions;
                return {};
            case TF_ALLOCATOR_OPTION_MAX_SPLIT_SIZE:
                *out_value = m_max_split_size;
                return {};
            case TF_ALLOCATOR_OPTION_GARBAGE_COLLECTION_THRESHOLD:
                *out_value = m_garbage_collection_threshold;
                return {};
            case TF_ALLOCATOR_OPTION_EXPANDABLE_SEGMENTS:
                *out_value = 0;
                return {};
        }

        return std::unexpected{ice::sonic::Status::from_message("SyclAllocator: unknown option")};
    }

    void get_stats(TF_AllocatorStats* out_stats, bool* out_success) noexcept override
    {
        *out_stats = m_stats;
        out_stats->struct_size = TF_ALLOCATOR_STRUCT_SIZE;
        out_stats->has_bytes_limit = m_memory_fraction < 1.0 ? 1 : 0;
        *out_success = true;
    }

    void reset_accumulated_stats() noexcept override
    {
        m_stats.num_allocs = 0;
    }

    void reset_peak_stats() noexcept override
    {
        m_stats.peak_bytes_in_use = m_stats.bytes_in_use;
        m_stats.peak_bytes_reserved = m_stats.bytes_reserved;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_snapshot(const TF_PoolId* pool_filter, ice::builder::Buffer& out_snapshot) noexcept override
    {
        (void)pool_filter;

        std::string report;
        report +=
            std::format("segments={} blocks_free={}\n", m_segments.size(), m_free_blocks.size());
        for (const Segment& segment: m_segments) {
            report += std::format(
                "segment base={} size={} blocks={}\n",
                std::bit_cast<uintptr_t>(segment.base),
                segment.size,
                segment.blocks.size()
            );
        }

        out_snapshot.assign_from_string(report.data(), report.size());
        return {};
    }

    void generate_pool_id(TF_PoolId* out_pool_id) noexcept override
    {
        *out_pool_id = TF_PoolId{.first = 0, .second = static_cast<int64_t>(++m_pool_id_sequence)};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_mem_pool_internal(
        const TF_PoolId* pool_id,
        bool is_user_created,
        TF_MemPool* out_pool
    ) noexcept
    {
        TF_PoolId assigned_id;
        if (pool_id != nullptr) {
            assigned_id = *pool_id;
        } else {
            generate_pool_id(&assigned_id);
        }

        auto owned = std::make_unique<SyclMemPool>(assigned_id, is_user_created);
        out_pool->plugin_data = owned.get();
        m_pools.push_back(std::move(owned));
        return {};
    }

    void destroy_mem_pool_internal(ice::builder::MemPool& pool) noexcept override
    {
        std::erase_if(
            m_pools,
            [&pool](const std::unique_ptr<SyclMemPool>& candidate)
            {
                return candidate.get() == &pool;
            }
        );
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    enable_peer_access(int peer_device_index) noexcept override
    {
        (void)peer_device_index;
        // SyclPlatform::can_access_peer already answers the query; USM pointers from a shared
        // context are already cross-device accessible once ext_oneapi_enable_peer_access runs,
        // which is out of scope for this reference allocator.
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> export_memory(
        const TF_DeviceMemoryBase* memory,
        TF_IpcMemoryHandle* out_handle
    ) noexcept override
    {
        (void)memory;
        (void)out_handle;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclAllocator: IPC export not implemented")
        };
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    open_memory(const TF_IpcMemoryHandle* handle, TF_DeviceMemoryBase* out_memory) noexcept override
    {
        (void)handle;
        (void)out_memory;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclAllocator: IPC import not implemented")
        };
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    close_memory(TF_DeviceMemoryBase* memory) noexcept override
    {
        (void)memory;
        return {};
    }

private:
    // A block is either free (in m_free_blocks and its segment's list) or allocated (handed out,
    // found only via the TF_DeviceMemoryBase::payload pointer). prev/next link split siblings
    // within the same segment, mirroring c10/XPUCachingAllocator.cpp's Block::splice.
    struct Block
    {
        void* ptr{nullptr};
        uint64_t size{0};
        uint64_t requested_size{0};
        bool allocated{false};
        Block* prev{nullptr};
        Block* next{nullptr};
        std::set<sycl::queue*> stream_uses;
    };

    struct Segment
    {
        void* base{nullptr};
        uint64_t size{0};
        std::vector<std::unique_ptr<Block>> blocks;
    };

    struct FreeBlockOrder
    {
        bool operator()(const Block* a, const Block* b) const noexcept
        {
            if (a->size != b->size) {
                return a->size < b->size;
            }
            return a->ptr < b->ptr;
        }
    };

    static sycl::queue& native_queue(ice::builder::Stream& stream) noexcept
    {
        return static_cast<SyclStream&>(stream).get_native_queue();
    }

    static uint64_t round_size(uint64_t size) noexcept
    {
        if (size < DEVICE_ALIGNMENT) {
            return DEVICE_ALIGNMENT;
        }
        return DEVICE_ALIGNMENT * ((size + DEVICE_ALIGNMENT - 1) / DEVICE_ALIGNMENT);
    }

    Block* find_free_block(uint64_t size, const sycl::queue* queue) noexcept
    {
        (void)queue;

        // Best-fit: the smallest free block at least `size`, matching
        // c10/XPUCachingAllocator.cpp's BlockComparatorSize ordering. Streams are not
        // partitioned into separate pools here (the reference allocator's whole point is
        // correctness, not the real allocator's per-stream pool split); record_stream/deallocate
        // already wait out cross-stream uses before a block is reused.
        auto search_key = Block{.size = size};
        auto it = m_free_blocks.lower_bound(&search_key);
        return it == m_free_blocks.end() ? nullptr : *it;
    }

    std::expected<void, ice::sonic::Status> grow_segment(uint64_t minimum_size) noexcept
    {
        const uint64_t segment_size = std::max(minimum_size, SMALL_SIZE);

        void* base = sycl::aligned_alloc_device(
            DEVICE_ALIGNMENT,
            segment_size,
            m_device.get_native_device(),
            m_context
        );

        if (base == nullptr) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclAllocator: device allocation failed")
            };
        }

        Segment segment{.base = base, .size = segment_size};
        auto block = std::make_unique<Block>();
        block->ptr = base;
        block->size = segment_size;
        m_free_blocks.insert(block.get());
        segment.blocks.push_back(std::move(block));
        m_segments.push_back(std::move(segment));

        m_stats.bytes_reserved += static_cast<int64_t>(segment_size);
        m_stats.peak_bytes_reserved = std::max(m_stats.peak_bytes_reserved, m_stats.bytes_reserved);

        return {};
    }

    void split_if_worthwhile(Block& block, uint64_t requested_size) noexcept
    {
        if (m_no_split || block.size - requested_size < DEVICE_ALIGNMENT) {
            m_free_blocks.erase(&block);
            return;
        }

        m_free_blocks.erase(&block);

        Segment* owning_segment = find_segment_containing(block.ptr);
        if (owning_segment == nullptr) {
            return;
        }

        auto remainder = std::make_unique<Block>();
        remainder->ptr = static_cast<std::byte*>(block.ptr) + requested_size;
        remainder->size = block.size - requested_size;
        remainder->prev = &block;
        remainder->next = block.next;
        if (block.next != nullptr) {
            block.next->prev = remainder.get();
        }
        block.next = remainder.get();
        block.size = requested_size;

        m_free_blocks.insert(remainder.get());
        owning_segment->blocks.push_back(std::move(remainder));
    }

    void coalesce(Block& block) noexcept
    {
        Block* merged = &block;

        if (merged->next != nullptr && !merged->next->allocated) {
            merge_with_next(*merged);
        }
        if (merged->prev != nullptr && !merged->prev->allocated) {
            merged = merged->prev;
            merge_with_next(*merged);
        }

        m_free_blocks.insert(merged);
    }

    void merge_with_next(Block& block) noexcept
    {
        Block* next = block.next;
        if (next == nullptr) {
            return;
        }

        m_free_blocks.erase(next);

        block.size += next->size;
        block.next = next->next;
        if (next->next != nullptr) {
            next->next->prev = &block;
        }

        Segment* owning_segment = find_segment_containing(block.ptr);
        if (owning_segment != nullptr) {
            std::erase_if(
                owning_segment->blocks,
                [next](const std::unique_ptr<Block>& candidate)
                {
                    return candidate.get() == next;
                }
            );
        }
    }

    Segment* find_segment_containing(const void* pointer) noexcept
    {
        for (Segment& segment: m_segments) {
            auto* base = static_cast<std::byte*>(segment.base);
            if (pointer >= base && pointer < base + segment.size) {
                return &segment;
            }
        }
        return nullptr;
    }

    Block* find_block_containing(const void* pointer) noexcept
    {
        Segment* segment = find_segment_containing(pointer);
        if (segment == nullptr) {
            return nullptr;
        }

        for (const std::unique_ptr<Block>& block: segment->blocks) {
            auto* base = static_cast<std::byte*>(block->ptr);
            if (pointer >= base && pointer < base + block->size) {
                return block.get();
            }
        }
        return nullptr;
    }

    std::expected<void, ice::sonic::Status> allocate_unpooled(
        uint64_t size,
        TF_MemorySpace memory_space,
        TF_DeviceMemoryBase* out_memory
    ) noexcept
    {
        void* pointer = memory_space == TF_MEMORY_SPACE_HOST_PINNED
                            ? sycl::aligned_alloc_host(DEVICE_ALIGNMENT, size, m_context)
                            : sycl::aligned_alloc_shared(
                                  DEVICE_ALIGNMENT,
                                  size,
                                  m_device.get_native_device(),
                                  m_context
                              );

        if (pointer == nullptr) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclAllocator: host/unified allocation failed")
            };
        }

        *out_memory = TF_DeviceMemoryBase{
            .struct_size = TF_OFFSET_OF_END(TF_DeviceMemoryBase, payload),
            .ext = nullptr,
            .opaque = pointer,
            .size = size,
            .payload = 0
        };

        return {};
    }

    sycl::context& m_context;
    SyclDevice& m_device;
    std::vector<Segment> m_segments;
    std::set<Block*, FreeBlockOrder> m_free_blocks;
    std::vector<std::unique_ptr<SyclMemPool>> m_pools;
    TF_AllocatorStats m_stats{.struct_size = TF_ALLOCATOR_STRUCT_SIZE};
    double m_memory_fraction{1.0};
    bool m_no_split{false};
    bool m_use_on_oom{false};
    int64_t m_round_up_power2_divisions{0};
    int64_t m_max_split_size{-1};
    int64_t m_garbage_collection_threshold{0};
    uint64_t m_pool_id_sequence{0};
};

} // namespace sycl_backend
