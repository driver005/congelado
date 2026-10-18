// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_stream_executor_sonic:allocator;

import std;
import :allocator_option;
import :allocator_stats;
import :device_memory_base;
import :ipc_memory_handle;
import :mem_pool;
import :memory_space;
import :pool_id;
import :stream;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Allocator : public ice::sonic::Runtime<::TF_AllocatorOps, ::TF_Allocator>
{
public:
    Allocator(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    Allocator(
        const ice::sonic::Registration& registry,
        const ::TF_Allocator& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit Allocator(const ::TF_AllocatorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Allocator(const ::TF_AllocatorOps* ops, const ::TF_Allocator* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void allocate(
        uint64_t size,
        ice::sonic::MemorySpace memory_space,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->allocate(
            get_handle(),
            size,
            memory_space.get_value(),
            stream.get_handle(),
            out_memory == nullptr ? nullptr : out_memory->get_handle(),
            out_status.get_handle()
        );
    }

    void deallocate(ice::sonic::DeviceMemoryBase* memory) const noexcept
    {
        m_ops->deallocate(get_handle(), memory == nullptr ? nullptr : memory->get_handle());
    }

    void record_stream(
        const ice::sonic::DeviceMemoryBase* memory,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->record_stream(
            get_handle(),
            memory == nullptr ? nullptr : memory->get_handle(),
            stream.get_handle(),
            out_status.get_handle()
        );
    }

    void owns_pointer(const void* pointer, bool& out_owns) const noexcept
    {
        m_ops->owns_pointer(get_handle(), pointer, &out_owns);
    }

    void get_base_allocation(
        const void* pointer,
        void** out_base,
        std::uint64_t& out_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_base_allocation(
            get_handle(),
            pointer,
            out_base,
            &out_size,
            out_status.get_handle()
        );
    }

    void empty_cache(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->empty_cache(get_handle(), out_status.get_handle());
    }

    void set_memory_fraction(double fraction, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_memory_fraction(get_handle(), fraction, out_status.get_handle());
    }

    void get_memory_fraction(double& out_fraction) const noexcept
    {
        m_ops->get_memory_fraction(get_handle(), &out_fraction);
    }

    void set_option(
        ice::sonic::AllocatorOption option,
        int64_t value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_option(get_handle(), option.get_value(), value, out_status.get_handle());
    }

    void get_option(
        ice::sonic::AllocatorOption option,
        std::int64_t& out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_option(get_handle(), option.get_value(), &out_value, out_status.get_handle());
    }

    void get_stats(ice::sonic::AllocatorStats* out_stats, bool& out_success) const noexcept
    {
        m_ops->get_stats(
            get_handle(),
            out_stats == nullptr ? nullptr : out_stats->get_handle(),
            &out_success
        );
    }

    void reset_accumulated_stats() const noexcept
    {
        m_ops->reset_accumulated_stats(get_handle());
    }

    void reset_peak_stats() const noexcept
    {
        m_ops->reset_peak_stats(get_handle());
    }

    void get_snapshot(
        const ice::sonic::PoolId* pool_filter,
        const ice::sonic::Buffer& out_snapshot,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_snapshot(
            get_handle(),
            pool_filter == nullptr ? nullptr : pool_filter->get_handle(),
            out_snapshot.get_handle(),
            out_status.get_handle()
        );
    }

    void generate_pool_id(ice::sonic::PoolId* out_pool_id) const noexcept
    {
        m_ops->generate_pool_id(
            get_handle(),
            out_pool_id == nullptr ? nullptr : out_pool_id->get_handle()
        );
    }

    void create_mem_pool_internal(
        const ice::sonic::PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::MemPool& out_pool,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_mem_pool_internal(
            get_handle(),
            pool_id == nullptr ? nullptr : pool_id->get_handle(),
            is_user_created,
            out_pool.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_mem_pool_internal(const ice::sonic::MemPool& pool) const noexcept
    {
        m_ops->destroy_mem_pool_internal(get_handle(), pool.get_handle());
    }

    void enable_peer_access(
        int peer_device_index,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->enable_peer_access(get_handle(), peer_device_index, out_status.get_handle());
    }

    void export_memory(
        const ice::sonic::DeviceMemoryBase* memory,
        ice::sonic::IpcMemoryHandle* out_handle,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->export_memory(
            get_handle(),
            memory == nullptr ? nullptr : memory->get_handle(),
            out_handle == nullptr ? nullptr : out_handle->get_handle(),
            out_status.get_handle()
        );
    }

    void open_memory(
        const ice::sonic::IpcMemoryHandle* handle,
        ice::sonic::DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->open_memory(
            get_handle(),
            handle == nullptr ? nullptr : handle->get_handle(),
            out_memory == nullptr ? nullptr : out_memory->get_handle(),
            out_status.get_handle()
        );
    }

    void close_memory(
        ice::sonic::DeviceMemoryBase* memory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->close_memory(
            get_handle(),
            memory == nullptr ? nullptr : memory->get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
