module;

#include "include/c/extern/stream_executor/allocator.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;
import :stream;
import :mem_pool;
import :allocator_block;
import :allocator_block_pool;
import :allocator_expandable_segment;
import :allocator_stats;
import :allocator_trace;
import :allocator_snapshot;
import :allocator_ipc_memory;
import :allocator_host_cache;

export namespace aten_xpu {

class SyclAllocator : public ice::builder::TF_AllocatorOps
{
public:
    static constexpr std::size_t k_min_block_size = 512;
    static constexpr std::size_t k_small_size = 1U << 20U;
    static constexpr std::size_t k_small_buffer = 2U << 20U;
    static constexpr std::size_t k_min_large_alloc = 10U << 20U;
    static constexpr std::size_t k_round_large = 2U << 20U;
    static constexpr std::size_t k_large_segment = 20U << 20U;
    static constexpr std::size_t k_large_buffer = 20U << 20U;

    using PeerAccessCallback = std::function<bool(int, int)>;
    using Kind = SyclAllocatorStats::Kind;
    using Action = SyclAllocatorTrace::Action;

    explicit SyclAllocator(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_AllocatorOps{
            ops.getBufferOps(),
            ops.getMemPoolOps(),
            ops.getStatusOps(),
            ops.getStreamOps()
        },
        m_status{ops},
        m_small_blocks{true, TF_PoolId{}},
        m_large_blocks{false, TF_PoolId{}}
    {
    }

    ~SyclAllocator() override { release(); }

    SyclAllocator(const SyclAllocator&) = delete;
    SyclAllocator& operator=(const SyclAllocator&) = delete;
    SyclAllocator(SyclAllocator&&) = delete;
    SyclAllocator& operator=(SyclAllocator&&) = delete;

    static void create(::TF_Allocator* handle)
    {

        auto* allocator = new SyclAllocator{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *allocator);

    }

    void bind(
        const sycl::context& context,
        const sycl::device& device,
        int device_index,
        PeerAccessCallback enable_peer
    )
    {

        m_context = context;
        m_device = device;
        m_device_index = device_index;
        m_enable_peer = std::move(enable_peer);
        m_default_queue.emplace(context, device, sycl::property_list{sycl::property::queue::in_order{}});
        m_host_cache = std::make_unique<SyclHostCache>(context);
        m_ipc_memory = std::make_unique<SyclIpcMemory>(context, device);
        m_device_total = device.get_info<sycl::info::device::global_mem_size>();

    }

    void release() noexcept
    {

        if (!m_default_queue) {
            return;
        }
        try {
            synchronize_and_free_events(std::nullopt);
            release_blocks(m_small_blocks);
            release_blocks(m_large_blocks);
        } catch (const sycl::exception&) {
        }
        for (const auto& [pointer, block]: m_allocated) {
            delete block;
        }
        m_allocated.clear();
        m_registered_pools.clear();
        m_host_cache.reset();
        m_ipc_memory.reset();
        m_default_queue.reset();

    }

    void destroy() noexcept override { delete this; }

    void allocate(
        uint64_t size,
        TF_MemorySpace memory_space,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            *out_memory = TF_DeviceMemoryBase{.struct_size = sizeof(TF_DeviceMemoryBase)};
            out_memory->size = size;

            if (memory_space == TF_MEMORY_SPACE_HOST_PINNED) {
                out_memory->opaque = m_host_cache->allocate(size);
            } else if (memory_space == TF_MEMORY_SPACE_UNIFIED) {
                out_memory->opaque = sycl::aligned_alloc_shared(k_min_block_size, size, m_device, m_context);
                if (out_memory->opaque != nullptr) {
                    m_unified.insert(out_memory->opaque);
                }
            } else if (auto* block = malloc(size, stream); block != nullptr) {
                out_memory->opaque = block->getPointer();
                out_memory->payload = reinterpret_cast<std::uintptr_t>(block);
            }

            if (out_memory->opaque == nullptr) {
                m_stats.record_out_of_memory();
                m_trace.record(Action::out_of_memory, nullptr, size, nullptr, TF_PoolId{});
                m_status.fail(out_status, TF_RESOURCE_EXHAUSTED, out_of_memory_message(size));
            }
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void deallocate(TF_DeviceMemoryBase* memory) noexcept override
    {

        if (memory == nullptr || memory->opaque == nullptr) {
            return;
        }

        if (memory->payload != 0) {
            free(reinterpret_cast<SyclBlock*>(memory->payload));
        } else if (!m_host_cache->deallocate(memory->opaque) && m_unified.erase(memory->opaque) != 0) {
            sycl::free(memory->opaque, m_context);
        }
        memory->opaque = nullptr;
        memory->payload = 0;

    }

    void record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        auto& queue = resolve_queue(stream);
        if (memory->payload == 0) {
            m_host_cache->record_stream(memory->opaque, queue);
            return;
        }

        auto* block = reinterpret_cast<SyclBlock*>(memory->payload);
        if (block->getQueue() != &queue) {
            block->addStreamUse(&queue);
        }

    }

    void owns_pointer(const void* pointer, _Bool* out_owns) noexcept override
    {

        *out_owns = find_allocated_block(pointer) != nullptr || m_host_cache->owns(pointer);

    }

    void get_base_allocation(
        const void* pointer,
        void** out_base,
        uint64_t* out_size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        const auto* block = find_allocated_block(pointer);
        if (block == nullptr) {
            m_status.fail(out_status, TF_NOT_FOUND, "pointer not owned by this allocator");
            return;
        }

        while (block->getPrevious() != nullptr) {
            block = block->getPrevious();
        }
        *out_base = block->getPointer();
        uint64_t total = 0;
        for (; block != nullptr; block = block->getNext()) {
            total += block->getSize();
        }
        *out_size = total;

    }

    void empty_cache(const ice::sonic::Status& out_status) noexcept override
    {

        try {
            release_cached_blocks(std::nullopt);
            m_host_cache->empty_cache();
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void set_memory_fraction(double fraction, const ice::sonic::Status& out_status) noexcept override
    {

        if (fraction < 0.0 || fraction > 1.0) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "memory fraction must be in [0, 1]");
            return;
        }

        const bool integrated = m_device.has(sycl::aspect::ext_oneapi_is_integrated_gpu);
        const double usable = integrated ? 0.94 : 0.98;
        m_memory_fraction = fraction;
        m_allowed_maximum = static_cast<std::size_t>(fraction * usable * static_cast<double>(m_device_total));
        m_has_fraction = true;
        m_stats.setBytesLimit(static_cast<int64_t>(m_allowed_maximum));

    }

    void get_memory_fraction(double* out_fraction) noexcept override { *out_fraction = m_memory_fraction; }

    void set_option(TF_AllocatorOption option, int64_t value, const ice::sonic::Status& out_status)
        noexcept override
    {

        switch (option) {
            case TF_ALLOCATOR_OPTION_EXPANDABLE_SEGMENTS:
                m_use_expandable = value != 0;
                return;
            case TF_ALLOCATOR_OPTION_NO_SPLIT:
                m_no_split = value != 0;
                return;
            case TF_ALLOCATOR_OPTION_USE_ON_OOM:
                m_use_on_oom = value != 0;
                return;
            case TF_ALLOCATOR_OPTION_MAX_SPLIT_SIZE:
                m_max_split_size =
                    value <= 0 ? std::numeric_limits<std::size_t>::max() : static_cast<std::size_t>(value);
                return;
            case TF_ALLOCATOR_OPTION_GARBAGE_COLLECTION_THRESHOLD:
                m_garbage_collection_threshold = static_cast<double>(value) / 100.0;
                return;
            case TF_ALLOCATOR_OPTION_ROUND_UP_POWER2_DIVISIONS:
                m_round_up_power2_divisions = static_cast<std::size_t>(std::max<int64_t>(value, 0));
                return;
        }
        m_status.fail(out_status, TF_INVALID_ARGUMENT, "unknown allocator option");

    }

    void get_option(TF_AllocatorOption option, int64_t* out_value, const ice::sonic::Status& out_status)
        noexcept override
    {

        switch (option) {
            case TF_ALLOCATOR_OPTION_EXPANDABLE_SEGMENTS:
                *out_value = m_use_expandable ? 1 : 0;
                return;
            case TF_ALLOCATOR_OPTION_NO_SPLIT:
                *out_value = m_no_split ? 1 : 0;
                return;
            case TF_ALLOCATOR_OPTION_USE_ON_OOM:
                *out_value = m_use_on_oom ? 1 : 0;
                return;
            case TF_ALLOCATOR_OPTION_MAX_SPLIT_SIZE:
                *out_value = m_max_split_size == std::numeric_limits<std::size_t>::max()
                                 ? -1
                                 : static_cast<int64_t>(m_max_split_size);
                return;
            case TF_ALLOCATOR_OPTION_GARBAGE_COLLECTION_THRESHOLD:
                *out_value = static_cast<int64_t>(m_garbage_collection_threshold * 100.0);
                return;
            case TF_ALLOCATOR_OPTION_ROUND_UP_POWER2_DIVISIONS:
                *out_value = static_cast<int64_t>(m_round_up_power2_divisions);
                return;
        }
        m_status.fail(out_status, TF_INVALID_ARGUMENT, "unknown allocator option");

    }

    void get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) noexcept override
    {

        m_stats.fill(*out_stats, static_cast<int64_t>(largest_free_block()));
        *out_success = true;

    }

    void reset_accumulated_stats() noexcept override { m_stats.reset_accumulated(); }

    void reset_peak_stats() noexcept override { m_stats.reset_peak(); }

    void get_snapshot(
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            m_snapshot.begin(m_device_index);
            for_each_block(
                pool_filter,
                [this](const SyclBlock& block)
                {

                    if (block.getPrevious() == nullptr) {
                        m_snapshot.add_segment(block);
                    }

                }
            );
            m_trace.record(
                Action::snapshot,
                nullptr,
                m_snapshot.getTotalActive(),
                nullptr,
                pool_filter == nullptr ? TF_PoolId{} : *pool_filter
            );
            const auto text = m_snapshot.finish(m_trace);
            out_snapshot.assign_from_string(text.data(), text.size());
        } catch (const std::exception& error) {
            m_status.fail(out_status, TF_INTERNAL, error.what());
        }

    }

    void generate_pool_id(TF_PoolId* out_pool_id) noexcept override
    {

        *out_pool_id = TF_PoolId{.first = 0, .second = static_cast<int64_t>(++m_pool_sequence)};

    }

    void create_mem_pool_internal(
        const TF_PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::TF_MemPoolOps& out_pool,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        TF_PoolId assigned{};
        if (pool_id != nullptr) {
            assigned = *pool_id;
        } else if (is_user_created) {
            generate_pool_id(&assigned);
        } else {
            assigned = TF_PoolId{.first = static_cast<int64_t>(++m_graph_pool_sequence), .second = 0};
        }

        if (auto existing = find_pool(assigned)) {
            existing->get().increment_use();
            return;
        }

        auto& pool = SyclHandle::resolve<SyclMemPool>(out_pool);
        pool.bind(assigned, is_user_created);
        m_registered_pools.emplace_back(pool);

    }

    void destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) noexcept override
    {

        auto& target = SyclHandle::resolve<SyclMemPool>(pool);
        try {
            release_cached_blocks(target.getPoolId());
        } catch (const sycl::exception&) {
        }
        std::erase_if(
            m_registered_pools,
            [&target](const std::reference_wrapper<SyclMemPool>& candidate)
            {

                return &candidate.get() == &target;

            }
        );

    }

    void enable_peer_access(int peer_device_index, const ice::sonic::Status& out_status) noexcept override
    {

        if (!m_enable_peer || !m_enable_peer(m_device_index, peer_device_index)) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "peer access is not supported");
            return;
        }
        m_peers.insert(peer_device_index);

    }

    void export_memory(
        const TF_DeviceMemoryBase* memory,
        TF_IpcMemoryHandle* out_handle,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (memory->payload == 0) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "only device memory can be exported");
            return;
        }

        const auto* block = reinterpret_cast<const SyclBlock*>(memory->payload);
        if (block->getExpandableSegment() != nullptr) {
            m_status.fail(out_status, TF_UNIMPLEMENTED, "expandable segments cannot be shared");
            return;
        }

        const auto* head = block;
        while (head->getPrevious() != nullptr) {
            head = head->getPrevious();
        }

        try {
            m_ipc_memory->export_handle(
                head->getPointer(),
                block->getPointer() - head->getPointer(),
                block->getSize(),
                *out_handle
            );
        } catch (const std::exception& error) {
            m_status.fail(out_status, TF_INTERNAL, error.what());
        }

    }

    void open_memory(
        const TF_IpcMemoryHandle* handle,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            *out_memory = TF_DeviceMemoryBase{.struct_size = sizeof(TF_DeviceMemoryBase)};
            out_memory->opaque = m_ipc_memory->open(*handle);
            out_memory->size = handle->allocation_size;
        } catch (const std::exception& error) {
            m_status.fail(out_status, TF_INTERNAL, error.what());
        }

    }

    void close_memory(TF_DeviceMemoryBase* memory, const ice::sonic::Status& out_status) noexcept override
    {

        if (!m_ipc_memory->close(memory->opaque)) {
            m_status.fail(out_status, TF_NOT_FOUND, "memory was not opened from an IPC handle");
            return;
        }
        memory->opaque = nullptr;

    }

    void setCapturing(bool capturing) noexcept
    {

        m_active_captures += capturing ? 1 : -1;
        if (m_active_captures == 0) {
            insert_deferred_events();
        }

    }

    SyclAllocatorTrace& getTrace() noexcept { return m_trace; }

    int getDeviceIndex() const noexcept { return m_device_index; }

private:
    sycl::queue& resolve_queue(const ice::sonic::TF_StreamOps& stream) noexcept
    {

        if (stream.get_handle()->plugin_data == nullptr) {
            return *m_default_queue;
        }
        return SyclHandle::resolve<SyclStream>(stream).getNativeQueue();

    }

    std::optional<std::reference_wrapper<SyclMemPool>> find_pool(const TF_PoolId& pool_id)
    {

        for (auto& pool: m_registered_pools) {
            if (same_pool(pool.get().getPoolId(), pool_id)) {
                return pool;
            }
        }
        return std::nullopt;

    }

    SyclBlockPool& select_pool(std::size_t size, ::TF_Stream* stream)
    {

        for (auto& pool: m_registered_pools) {
            if (pool.get().matches(stream)) {
                return size <= k_small_size ? pool.get().getSmallBlocks() : pool.get().getLargeBlocks();
            }
        }
        return size <= k_small_size ? m_small_blocks : m_large_blocks;

    }

    SyclBlock* malloc(std::size_t requested_size, const ice::sonic::TF_StreamOps& stream)
    {

        if (m_active_captures == 0) {
            process_events();
        }

        auto& queue = resolve_queue(stream);
        const auto size = round_size(requested_size);
        auto* pool = &select_pool(size, stream.get_handle());
        const auto allocation_size = segment_size_for(size);

        auto* block = take_free_block(*pool, &queue, size);
        if (block == nullptr) {
            block = allocate_segment(*pool, &queue, size, allocation_size, false);
        }
        if (block == nullptr && m_active_captures == 0) {
            block = try_mem_pool_fallback(&queue, size, pool);
        }
        if (block == nullptr && m_active_captures == 0) {
            release_cached_blocks(std::nullopt);
            block = allocate_segment(*pool, &queue, size, allocation_size, true);
        }
        if (block == nullptr) {
            return nullptr;
        }

        block = carve_block(*block, size, requested_size);
        garbage_collect();
        return block;

    }

    SyclBlock* take_free_block(SyclBlockPool& pool, sycl::queue* queue, std::size_t size)
    {

        auto* block = pool.take_best_fit(queue, size, m_use_expandable);
        if (block == nullptr) {
            return nullptr;
        }

        const bool oversize_request = size >= m_max_split_size;
        const bool oversize_block = block->getSize() >= m_max_split_size;
        if ((!oversize_request && oversize_block) ||
            (oversize_request && block->getSize() >= size + k_large_buffer))
        {
            pool.getBlocks().insert(block);
            return nullptr;
        }
        return block;

    }

    SyclBlock* try_mem_pool_fallback(sycl::queue* queue, std::size_t size, SyclBlockPool*& pool)
    {

        if (!pool->is_default()) {
            return nullptr;
        }
        for (auto& candidate: m_registered_pools) {
            if (!candidate.get().getUseOnOom() && !m_use_on_oom) {
                continue;
            }
            auto& fallback =
                size <= k_small_size ? candidate.get().getSmallBlocks() : candidate.get().getLargeBlocks();
            if (auto* block = take_free_block(fallback, queue, size)) {
                pool = &fallback;
                return block;
            }
        }
        return nullptr;

    }

    SyclBlock* allocate_segment(
        SyclBlockPool& pool,
        sycl::queue* queue,
        std::size_t size,
        std::size_t allocation_size,
        bool is_retry
    )
    {

        if (is_retry) {
            m_stats.record_alloc_retry();
        }
        if (m_has_fraction &&
            static_cast<std::size_t>(m_stats.getCurrent(Kind::reserved_bytes)) + allocation_size >
                m_allowed_maximum)
        {
            return nullptr;
        }

        if (m_use_expandable && pool.is_default()) {
            auto* block = allocate_expandable(pool, queue, size);
            if (block != nullptr) {
                pool.addAllocation();
            }
            return block;
        }

        void* pointer = allocate_primitive(pool, allocation_size);
        if (pointer == nullptr) {
            return nullptr;
        }

        pool.addAllocation();
        auto* block = new SyclBlock{queue, allocation_size, &pool, static_cast<std::byte*>(pointer)};
        m_stats.increase(Kind::segment, pool.getIsSmall(), 1);
        m_stats.increase(Kind::reserved_bytes, pool.getIsSmall(), static_cast<int64_t>(allocation_size));
        m_trace.record(Action::segment_alloc, pointer, allocation_size, queue, pool.getOwner());
        return block;

    }

    void* allocate_primitive(SyclBlockPool& pool, std::size_t size)
    {

        if (auto owner = find_pool(pool.getOwner()); owner && owner->get().getRawAllocate()) {
            return owner->get().getRawAllocate()(size);
        }
        return sycl::aligned_alloc_device(k_min_block_size, size, m_device, m_context);

    }

    void free_primitive(SyclBlockPool& pool, void* pointer)
    {

        if (auto owner = find_pool(pool.getOwner()); owner && owner->get().getRawDeallocate()) {
            owner->get().getRawDeallocate()(pointer);
            return;
        }
        sycl::free(pointer, m_context);

    }

    SyclBlock* allocate_expandable(SyclBlockPool& pool, sycl::queue* queue, std::size_t size)
    {

        auto* candidate = find_expandable_block(pool, queue, size);
        if (!candidate->getMapped() && !map_block(*candidate, std::min(candidate->getSize(), size))) {
            return nullptr;
        }

        while (candidate->getSize() < size) {
            auto* next = candidate->getNext();
            if (next == nullptr || !map_block(*next, std::min(size - candidate->getSize(), next->getSize()))) {
                return nullptr;
            }
            candidate = next;
        }
        pool.getBlocks().erase(candidate);
        return candidate;

    }

    SyclBlock* find_expandable_block(SyclBlockPool& pool, sycl::queue* queue, std::size_t size)
    {

        SyclBlock key{queue, 0};
        for (auto found = pool.getUnmapped().lower_bound(&key);
             found != pool.getUnmapped().end() && (*found)->getQueue() == queue;
             ++found)
        {
            auto* candidate = *found;
            if (candidate->getPrevious() != nullptr && candidate->getPrevious()->is_free()) {
                candidate = candidate->getPrevious();
            }
            std::size_t available = 0;
            for (auto* cursor = candidate; cursor != nullptr && cursor->is_free() && available < size;
                 cursor = cursor->getNext())
            {
                available += cursor->getSize();
            }
            if (available >= size) {
                return candidate;
            }
        }

        const auto segment_size = pool.getIsSmall() ? k_small_buffer : k_large_segment;
        auto& segment = *m_expandable_segments.emplace_back(
            std::make_unique<SyclExpandableSegment>(m_context, m_device, segment_size)
        );
        auto* candidate = new SyclBlock{queue, segment.getSize(), &pool, segment.getPointer()};
        candidate->setMapped(false);
        candidate->setExpandableSegment(&segment);
        pool.getUnmapped().insert(candidate);
        return candidate;

    }

    bool map_block(SyclBlock& block, std::size_t size)
    {

        auto& pool = *block.getPool();
        const auto mapped = block.getExpandableSegment()->map({block.getPointer(), size});
        if (mapped.empty()) {
            return false;
        }

        pool.getUnmapped().erase(&block);
        block.setMapped(true);
        if (mapped.size() < block.getSize()) {
            auto* remaining = new SyclBlock{
                block.getQueue(),
                block.getSize() - mapped.size(),
                &pool,
                block.getPointer() + mapped.size()
            };
            remaining->setMapped(false);
            remaining->setExpandableSegment(block.getExpandableSegment());
            remaining->splice(&block, block.getNext());
            pool.getUnmapped().insert(remaining);
            block.setSize(mapped.size());
        }

        try_merge(block, block.getPrevious(), pool);
        try_merge(block, block.getNext(), pool);
        pool.getBlocks().insert(&block);
        m_stats.increase(Kind::reserved_bytes, pool.getIsSmall(), static_cast<int64_t>(mapped.size()));
        m_trace.record(Action::segment_map, mapped.data(), mapped.size(), block.getQueue(), pool.getOwner());
        return true;

    }

    void unmap_block(SyclBlock& block)
    {

        auto& pool = *block.getPool();
        const auto unmapped = block.getExpandableSegment()->unmap({block.getPointer(), block.getSize()});
        if (unmapped.empty()) {
            return;
        }
        pool.getBlocks().erase(&block);

        const auto before_size = static_cast<std::size_t>(unmapped.data() - block.getPointer());
        if (before_size > 0) {
            auto* before = new SyclBlock{block.getQueue(), before_size, &pool, block.getPointer()};
            before->setExpandableSegment(block.getExpandableSegment());
            before->splice(block.getPrevious(), &block);
            pool.getBlocks().insert(before);
        }

        const auto after_size = block.getSize() - (before_size + unmapped.size());
        if (after_size > 0) {
            auto* after =
                new SyclBlock{block.getQueue(), after_size, &pool, unmapped.data() + unmapped.size()};
            after->setExpandableSegment(block.getExpandableSegment());
            after->splice(&block, block.getNext());
            pool.getBlocks().insert(after);
        }

        block.setPointer(unmapped.data());
        block.setSize(unmapped.size());
        block.setMapped(false);
        try_merge(block, block.getPrevious(), pool);
        try_merge(block, block.getNext(), pool);
        pool.getUnmapped().insert(&block);
        pool.remove_allocation();
        m_stats.decrease(Kind::reserved_bytes, pool.getIsSmall(), static_cast<int64_t>(unmapped.size()));
        m_trace.record(Action::segment_unmap, unmapped.data(), unmapped.size(), block.getQueue(), pool.getOwner());

    }

    bool should_split(const SyclBlock& block, std::size_t size) const
    {

        const auto& pool = *block.getPool();
        if (m_no_split) {
            return false;
        }
        if (!pool.is_default()) {
            for (const auto& candidate: m_registered_pools) {
                if (same_pool(candidate.get().getPoolId(), pool.getOwner()) && candidate.get().getNoSplit()) {
                    return false;
                }
            }
        }

        const auto remaining = block.getSize() - size;
        if (pool.getIsSmall() || m_use_expandable) {
            return remaining >= k_min_block_size;
        }
        return size < m_max_split_size && remaining > k_small_size;

    }

    SyclBlock* carve_block(SyclBlock& found, std::size_t size, std::size_t requested_size)
    {

        auto& pool = *found.getPool();
        const bool small_pool = pool.getIsSmall();
        const bool already_split = found.is_split();
        auto* block = &found;

        if (should_split(found, size)) {
            auto* remaining = &found;
            block = new SyclBlock{found.getQueue(), size, &pool, found.getPointer()};
            block->setExpandableSegment(remaining->getExpandableSegment());
            block->splice(remaining->getPrevious(), remaining);
            remaining->setPointer(remaining->getPointer() + size);
            remaining->setSize(remaining->getSize() - size);
            pool.getBlocks().insert(remaining);

            if (block->getExpandableSegment() == nullptr) {
                if (already_split) {
                    m_stats.decrease(Kind::inactive_split_bytes, small_pool, static_cast<int64_t>(block->getSize()));
                } else {
                    m_stats.increase(Kind::inactive_split, small_pool, 1);
                    m_stats.increase(Kind::inactive_split_bytes, small_pool, static_cast<int64_t>(remaining->getSize()));
                }
            }
        } else if (already_split && block->getExpandableSegment() == nullptr) {
            m_stats.decrease(Kind::inactive_split, small_pool, 1);
            m_stats.decrease(Kind::inactive_split_bytes, small_pool, static_cast<int64_t>(block->getSize()));
        }

        block->setAllocated(true);
        block->setRequestedSize(requested_size);
        m_allocated.emplace(block->getPointer(), block);

        const auto block_size = static_cast<int64_t>(block->getSize());
        m_stats.increase(Kind::allocation, small_pool, 1);
        m_stats.increase(Kind::active, small_pool, 1);
        m_stats.increase(Kind::allocated_bytes, small_pool, block_size);
        m_stats.increase(Kind::active_bytes, small_pool, block_size);
        m_stats.increase(Kind::requested_bytes, small_pool, static_cast<int64_t>(requested_size));
        m_stats.observe_allocation_size(static_cast<int64_t>(requested_size));
        m_trace.record(Action::alloc, block->getPointer(), requested_size, block->getQueue(), pool.getOwner());
        return block;

    }

    void free(SyclBlock* block)
    {

        const bool small_pool = block->getPool()->getIsSmall();
        block->setAllocated(false);
        m_allocated.erase(block->getPointer());
        m_stats.decrease(Kind::allocation, small_pool, 1);
        m_stats.decrease(Kind::allocated_bytes, small_pool, static_cast<int64_t>(block->getSize()));
        m_trace.record(
            Action::free_requested,
            block->getPointer(),
            block->getRequestedSize(),
            block->getQueue(),
            block->getPool()->getOwner()
        );

        if (block->getStreamUses().empty()) {
            free_block(*block);
        } else if (m_active_captures > 0) {
            m_deferred_blocks.insert(block);
        } else {
            insert_events(*block);
        }

    }

    void free_block(SyclBlock& block)
    {

        auto& pool = *block.getPool();
        const bool small_pool = pool.getIsSmall();
        const auto original_size = static_cast<int64_t>(block.getSize());
        const auto requested_size = static_cast<int64_t>(block.getRequestedSize());
        m_trace.record(Action::free_completed, block.getPointer(), block.getRequestedSize(), block.getQueue(), pool.getOwner());

        int64_t split_blocks_change = 0;
        int64_t split_bytes_change = 0;
        for (auto* candidate: {block.getPrevious(), block.getNext()}) {
            const auto subsumed = try_merge(block, candidate, pool);
            if (subsumed > 0) {
                split_blocks_change -= 1;
                split_bytes_change -= static_cast<int64_t>(subsumed);
            }
        }
        pool.getBlocks().insert(&block);
        if (block.is_split()) {
            split_blocks_change += 1;
            split_bytes_change += static_cast<int64_t>(block.getSize());
        }

        if (block.getExpandableSegment() == nullptr) {
            m_stats.increase(Kind::inactive_split, small_pool, split_blocks_change);
            m_stats.increase(Kind::inactive_split_bytes, small_pool, split_bytes_change);
        }
        m_stats.decrease(Kind::active, small_pool, 1);
        m_stats.decrease(Kind::active_bytes, small_pool, original_size);
        m_stats.decrease(Kind::requested_bytes, small_pool, requested_size);

    }

    std::size_t try_merge(SyclBlock& target, SyclBlock* source, SyclBlockPool& pool)
    {

        if (source == nullptr || !source->is_free() || target.getMapped() != source->getMapped()) {
            return 0;
        }

        if (target.getPrevious() == source) {
            target.setPointer(source->getPointer());
            target.setPrevious(source->getPrevious());
            if (target.getPrevious() != nullptr) {
                target.getPrevious()->setNext(&target);
            }
        } else {
            target.setNext(source->getNext());
            if (target.getNext() != nullptr) {
                target.getNext()->setPrevious(&target);
            }
        }

        const auto subsumed = source->getSize();
        target.setSize(target.getSize() + subsumed);
        if (source->getMapped()) {
            pool.getBlocks().erase(source);
        } else {
            pool.getUnmapped().erase(source);
        }
        delete source;
        return subsumed;

    }

    void insert_events(SyclBlock& block)
    {

        auto streams = std::move(block.getStreamUses());
        block.getStreamUses().clear();
        for (auto* queue: streams) {
            block.setEventCount(block.getEventCount() + 1);
            m_events[queue].emplace_back(queue->ext_oneapi_submit_barrier(), &block);
        }

    }

    void insert_deferred_events()
    {

        for (auto* block: m_deferred_blocks) {
            insert_events(*block);
        }
        m_deferred_blocks.clear();

    }

    void process_events()
    {

        insert_deferred_events();
        for (auto entry = m_events.begin(); entry != m_events.end();) {
            auto& pending = entry->second;
            while (!pending.empty()) {
                auto& [event, block] = pending.front();
                if (event.get_info<sycl::info::event::command_execution_status>() !=
                    sycl::info::event_command_status::complete)
                {
                    break;
                }
                block->setEventCount(block->getEventCount() - 1);
                if (block->getEventCount() == 0) {
                    free_block(*block);
                }
                pending.pop_front();
            }
            entry = pending.empty() ? m_events.erase(entry) : std::next(entry);
        }

    }

    void synchronize_and_free_events(std::optional<TF_PoolId> pool_id)
    {

        insert_deferred_events();
        for (auto& [queue, pending]: m_events) {
            for (auto& [event, block]: pending) {
                if (pool_id && !same_pool(block->getPool()->getOwner(), *pool_id)) {
                    continue;
                }
                event.wait();
                block->setEventCount(block->getEventCount() - 1);
                if (block->getEventCount() == 0) {
                    free_block(*block);
                }
            }
            std::erase_if(
                pending,
                [](const std::pair<sycl::event, SyclBlock*>& candidate)
                {

                    return candidate.second->getEventCount() == 0;

                }
            );
        }
        std::erase_if(
            m_events,
            [](const auto& entry)
            {

                return entry.second.empty();

            }
        );

    }

    void release_block(SyclBlock& block)
    {

        auto& pool = *block.getPool();
        m_trace.record(Action::segment_free, block.getPointer(), block.getSize(), block.getQueue(), pool.getOwner());
        free_primitive(pool, block.getPointer());
        pool.remove_allocation();
        pool.getBlocks().erase(&block);
        m_stats.decrease(Kind::segment, pool.getIsSmall(), 1);
        m_stats.decrease(Kind::reserved_bytes, pool.getIsSmall(), static_cast<int64_t>(block.getSize()));
        delete &block;

    }

    void release_blocks(SyclBlockPool& pool)
    {

        m_scratch_blocks.clear();
        for (auto* block: pool.getBlocks()) {
            if (block->getExpandableSegment() != nullptr || !block->is_split()) {
                m_scratch_blocks.push_back(block);
            }
        }
        for (auto* block: m_scratch_blocks) {
            if (block->getExpandableSegment() != nullptr) {
                unmap_block(*block);
            } else {
                release_block(*block);
            }
        }

    }

    void release_cached_blocks(std::optional<TF_PoolId> pool_id)
    {

        synchronize_and_free_events(pool_id);
        m_default_queue->wait_and_throw();

        if (!pool_id || (pool_id->first == 0 && pool_id->second == 0)) {
            release_blocks(m_large_blocks);
            release_blocks(m_small_blocks);
        }

        for (auto& candidate: m_registered_pools) {
            auto& pool = candidate.get();
            if (pool_id && !same_pool(pool.getPoolId(), *pool_id)) {
                continue;
            }
            if (pool.getUseCount() == 0) {
                release_blocks(pool.getSmallBlocks());
                release_blocks(pool.getLargeBlocks());
            }
        }

    }

    void garbage_collect()
    {

        if (!m_has_fraction || m_garbage_collection_threshold <= 0.0) {
            return;
        }

        const auto threshold =
            static_cast<int64_t>(m_garbage_collection_threshold * static_cast<double>(m_allowed_maximum));
        for (auto* pool: {&m_large_blocks, &m_small_blocks}) {
            m_scratch_blocks.assign(pool->getBlocks().begin(), pool->getBlocks().end());
            for (auto* block: m_scratch_blocks) {
                if (m_stats.getCurrent(Kind::reserved_bytes) <= threshold) {
                    return;
                }
                if (!block->is_split() && block->getExpandableSegment() == nullptr) {
                    release_block(*block);
                }
            }
        }

    }

    template<typename Visitor>
    void for_each_block(const TF_PoolId* pool_filter, Visitor&& visitor)
    {

        const auto visit_pool = [&visitor](SyclBlockPool& pool)
        {

            for (const auto* block: pool.getBlocks()) {
                visitor(*block);
            }

        };

        if (pool_filter == nullptr) {
            visit_pool(m_small_blocks);
            visit_pool(m_large_blocks);
        }
        for (auto& candidate: m_registered_pools) {
            if (pool_filter == nullptr || same_pool(candidate.get().getPoolId(), *pool_filter)) {
                visit_pool(candidate.get().getSmallBlocks());
                visit_pool(candidate.get().getLargeBlocks());
            }
        }
        for (const auto& [pointer, block]: m_allocated) {
            if (pool_filter == nullptr || same_pool(block->getPool()->getOwner(), *pool_filter)) {
                visitor(*block);
            }
        }

    }

    const SyclBlock* find_allocated_block(const void* pointer) const
    {

        const auto* address = static_cast<const std::byte*>(pointer);
        auto found = m_allocated.upper_bound(const_cast<std::byte*>(address));
        if (found == m_allocated.begin()) {
            return nullptr;
        }
        --found;
        const auto* block = found->second;
        return address < block->getPointer() + block->getSize() ? block : nullptr;

    }

    std::size_t largest_free_block()
    {

        std::size_t largest = 0;
        for (auto* pool: {&m_small_blocks, &m_large_blocks}) {
            for (const auto* block: pool->getBlocks()) {
                largest = std::max(largest, block->getSize());
            }
        }
        return largest;

    }

    std::size_t round_size(std::size_t size) const noexcept
    {

        if (size <= k_min_block_size) {
            return k_min_block_size;
        }
        if (m_round_up_power2_divisions > 1 && size > k_min_block_size * 2) {
            return round_up_power2_division(size, m_round_up_power2_divisions);
        }
        return k_min_block_size * ((size + k_min_block_size - 1) / k_min_block_size);

    }

    static std::size_t round_up_power2_division(std::size_t size, std::size_t divisions) noexcept
    {

        if (std::has_single_bit(size)) {
            return size;
        }
        const auto floor = std::bit_floor(size);
        const auto division = floor >> (std::bit_width(divisions) - 1);
        if (division == 0) {
            return floor << 1U;
        }
        const auto rounded_floor = size & ~(division - 1);
        return rounded_floor == size ? size : rounded_floor + division;

    }

    static std::size_t segment_size_for(std::size_t size) noexcept
    {

        if (size <= k_small_size) {
            return k_small_buffer;
        }
        if (size < k_min_large_alloc) {
            return k_large_segment;
        }
        return k_round_large * ((size + k_round_large - 1) / k_round_large);

    }

    static bool same_pool(const TF_PoolId& left, const TF_PoolId& right) noexcept
    {

        return left.first == right.first && left.second == right.second;

    }

    std::string out_of_memory_message(std::size_t size) const
    {

        const auto allocated = m_stats.getCurrent(Kind::allocated_bytes);
        const auto reserved = m_stats.getCurrent(Kind::reserved_bytes);
        return std::format(
            "XPU out of memory. Tried to allocate {} bytes on device {} with {} total bytes; "
            "{} bytes allocated, {} bytes reserved but unallocated",
            size,
            m_device_index,
            m_device_total,
            allocated,
            reserved - allocated
        );

    }

    SyclStatus m_status;
    sycl::context m_context;
    sycl::device m_device;
    int m_device_index{-1};
    std::size_t m_device_total{0};
    std::optional<sycl::queue> m_default_queue;
    PeerAccessCallback m_enable_peer;
    std::set<int> m_peers;
    SyclBlockPool m_small_blocks;
    SyclBlockPool m_large_blocks;
    std::map<std::byte*, SyclBlock*> m_allocated;
    std::set<void*> m_unified;
    std::set<SyclBlock*> m_deferred_blocks;
    std::map<sycl::queue*, std::deque<std::pair<sycl::event, SyclBlock*>>> m_events;
    std::vector<std::reference_wrapper<SyclMemPool>> m_registered_pools;
    std::vector<std::unique_ptr<SyclExpandableSegment>> m_expandable_segments;
    std::vector<SyclBlock*> m_scratch_blocks;
    std::unique_ptr<SyclHostCache> m_host_cache;
    std::unique_ptr<SyclIpcMemory> m_ipc_memory;
    SyclAllocatorStats m_stats;
    SyclAllocatorTrace m_trace;
    SyclAllocatorSnapshot m_snapshot;
    double m_memory_fraction{1.0};
    std::size_t m_allowed_maximum{0};
    bool m_has_fraction{false};
    bool m_use_expandable{false};
    bool m_no_split{false};
    bool m_use_on_oom{false};
    std::size_t m_max_split_size{std::numeric_limits<std::size_t>::max()};
    double m_garbage_collection_threshold{0.0};
    std::size_t m_round_up_power2_divisions{0};
    std::uint64_t m_pool_sequence{0};
    std::uint64_t m_graph_pool_sequence{0};
    int m_active_captures{0};
};

} // namespace aten_xpu
