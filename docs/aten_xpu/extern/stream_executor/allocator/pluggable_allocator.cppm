module;

#include "include/c/extern/stream_executor/allocator.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_pluggable;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;
import :stream;

export namespace aten_xpu {

class SyclPluggableAllocator : public ice::builder::TF_AllocatorOps
{
public:
    using AllocateFunction = std::function<void*(std::size_t, int, sycl::queue*)>;
    using DeallocateFunction = std::function<void(void*, std::size_t, int, sycl::queue*)>;
    using RecordStreamFunction = std::function<void(void*, sycl::queue*)>;

    explicit SyclPluggableAllocator(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_AllocatorOps{
            ops.getBufferOps(),
            ops.getMemPoolOps(),
            ops.getStatusOps(),
            ops.getStreamOps()
        },
        m_status{ops}
    {
    }

    ~SyclPluggableAllocator() override = default;
    SyclPluggableAllocator(const SyclPluggableAllocator&) = delete;
    SyclPluggableAllocator& operator=(const SyclPluggableAllocator&) = delete;
    SyclPluggableAllocator(SyclPluggableAllocator&&) = delete;
    SyclPluggableAllocator& operator=(SyclPluggableAllocator&&) = delete;

    void setFunctions(AllocateFunction allocate, DeallocateFunction deallocate)
    {

        m_allocate = std::move(allocate);
        m_deallocate = std::move(deallocate);

    }

    void setRecordStream(RecordStreamFunction record_stream)
    {

        m_record_stream = std::move(record_stream);

    }

    void bind(int device_index, sycl::queue& default_queue) noexcept
    {

        m_device_index = device_index;
        m_default_queue = default_queue;

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

        if (memory_space != TF_MEMORY_SPACE_DEVICE || !m_allocate) {
            m_status.fail(out_status, TF_UNIMPLEMENTED, "pluggable allocator serves device memory only");
            return;
        }

        auto* queue = resolve_queue(stream);
        void* pointer = m_allocate(size, m_device_index, queue);
        if (pointer == nullptr) {
            m_status.fail(out_status, TF_RESOURCE_EXHAUSTED, "custom allocator returned null");
            return;
        }

        m_metadata.emplace(pointer, std::pair{static_cast<std::size_t>(size), queue});
        *out_memory = TF_DeviceMemoryBase{.struct_size = sizeof(TF_DeviceMemoryBase)};
        out_memory->opaque = pointer;
        out_memory->size = size;

    }

    void deallocate(TF_DeviceMemoryBase* memory) noexcept override
    {

        auto found = m_metadata.find(memory->opaque);
        if (found == m_metadata.end()) {
            return;
        }
        m_deallocate(memory->opaque, found->second.first, m_device_index, found->second.second);
        m_metadata.erase(found);
        memory->opaque = nullptr;

    }

    void record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        if (m_record_stream) {
            m_record_stream(memory->opaque, resolve_queue(stream));
        }

    }

    void owns_pointer(const void* pointer, _Bool* out_owns) noexcept override
    {

        *out_owns = m_metadata.contains(const_cast<void*>(pointer));

    }

    void get_base_allocation(
        const void* pointer,
        void** out_base,
        uint64_t* out_size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        const auto found = m_metadata.find(const_cast<void*>(pointer));
        if (found == m_metadata.end()) {
            m_status.fail(out_status, TF_NOT_FOUND, "pointer not owned by this allocator");
            return;
        }
        *out_base = found->first;
        *out_size = found->second.first;

    }

    void empty_cache(const ice::sonic::Status& out_status) noexcept override
    {

        m_status.fail_unimplemented(out_status, "empty_cache");

    }

    void set_memory_fraction(double fraction, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(fraction);
        m_status.fail_unimplemented(out_status, "set_memory_fraction");

    }

    void get_memory_fraction(double* out_fraction) noexcept override { *out_fraction = 1.0; }

    void set_option(TF_AllocatorOption option, int64_t value, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(option);
        static_cast<void>(value);
        m_status.fail_unimplemented(out_status, "set_option");

    }

    void get_option(
        TF_AllocatorOption option,
        int64_t* out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(option);
        static_cast<void>(out_value);
        m_status.fail_unimplemented(out_status, "get_option");

    }

    void get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) noexcept override
    {

        static_cast<void>(out_stats);
        *out_success = false;

    }

    void reset_accumulated_stats() noexcept override {}

    void reset_peak_stats() noexcept override {}

    void get_snapshot(
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(pool_filter);
        static_cast<void>(out_snapshot);
        m_status.fail_unimplemented(out_status, "get_snapshot");

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

        static_cast<void>(pool_id);
        static_cast<void>(is_user_created);
        static_cast<void>(out_pool);
        m_status.fail_unimplemented(out_status, "create_mem_pool_internal");

    }

    void destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) noexcept override
    {

        static_cast<void>(pool);

    }

    void enable_peer_access(int peer_device_index, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(peer_device_index);
        m_status.fail_unimplemented(out_status, "enable_peer_access");

    }

    void export_memory(
        const TF_DeviceMemoryBase* memory,
        TF_IpcMemoryHandle* out_handle,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(memory);
        static_cast<void>(out_handle);
        m_status.fail_unimplemented(out_status, "export_memory");

    }

    void open_memory(
        const TF_IpcMemoryHandle* handle,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(handle);
        static_cast<void>(out_memory);
        m_status.fail_unimplemented(out_status, "open_memory");

    }

    void close_memory(TF_DeviceMemoryBase* memory, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(memory);
        m_status.fail_unimplemented(out_status, "close_memory");

    }

private:
    sycl::queue* resolve_queue(const ice::sonic::TF_StreamOps& stream) noexcept
    {

        if (stream.get_handle()->plugin_data == nullptr) {
            return m_default_queue ? &m_default_queue->get() : nullptr;
        }
        return &SyclHandle::resolve<SyclStream>(stream).getNativeQueue();

    }

    SyclStatus m_status;
    AllocateFunction m_allocate;
    DeallocateFunction m_deallocate;
    RecordStreamFunction m_record_stream;
    int m_device_index{-1};
    std::optional<std::reference_wrapper<sycl::queue>> m_default_queue;
    std::unordered_map<void*, std::pair<std::size_t, sycl::queue*>> m_metadata;
    std::uint64_t m_pool_sequence{0};
};

} // namespace aten_xpu
