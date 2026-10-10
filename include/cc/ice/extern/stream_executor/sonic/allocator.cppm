// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/allocator.h"

export module cc_ice_extern_stream_executor_sonic:allocator;

import std;
import :mem_pool;
import :stream;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_AllocatorOps : public ice::sonic::Runtime<::TF_AllocatorOps, ::TF_Allocator>
{
public:
    TF_AllocatorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_AllocatorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Allocator* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_AllocatorOps(const ::TF_AllocatorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_AllocatorOps(const ::TF_AllocatorOps* ops, ::TF_Allocator* handle) noexcept :
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
        TF_MemorySpace memory_space,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->allocate(
            get_handle(),
            size,
            memory_space,
            stream.get_handle(),
            out_memory,
            out_status.get_handle()
        );
    }

    void deallocate(TF_DeviceMemoryBase* memory) const noexcept
    {
        m_ops->deallocate(get_handle(), memory);
    }

    void record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->record_stream(get_handle(), memory, stream.get_handle(), out_status.get_handle());
    }

    void owns_pointer(const void* pointer, _Bool* out_owns) const noexcept
    {
        m_ops->owns_pointer(get_handle(), pointer, out_owns);
    }

    void get_base_allocation(
        const void* pointer,
        void** out_base,
        uint64_t* out_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_base_allocation(
            get_handle(),
            pointer,
            out_base,
            out_size,
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

    void get_memory_fraction(double* out_fraction) const noexcept
    {
        m_ops->get_memory_fraction(get_handle(), out_fraction);
    }

    void set_option(
        TF_AllocatorOption option,
        int64_t value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_option(get_handle(), option, value, out_status.get_handle());
    }

    void get_option(
        TF_AllocatorOption option,
        int64_t* out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_option(get_handle(), option, out_value, out_status.get_handle());
    }

    void get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) const noexcept
    {
        m_ops->get_stats(get_handle(), out_stats, out_success);
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
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_snapshot(
            get_handle(),
            pool_filter,
            out_snapshot.get_handle(),
            out_status.get_handle()
        );
    }

    void generate_pool_id(TF_PoolId* out_pool_id) const noexcept
    {
        m_ops->generate_pool_id(get_handle(), out_pool_id);
    }

    void create_mem_pool_internal(
        const TF_PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::TF_MemPoolOps& out_pool,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_mem_pool_internal(
            get_handle(),
            pool_id,
            is_user_created,
            out_pool.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) const noexcept
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
        const TF_DeviceMemoryBase* memory,
        TF_IpcMemoryHandle* out_handle,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->export_memory(get_handle(), memory, out_handle, out_status.get_handle());
    }

    void open_memory(
        const TF_IpcMemoryHandle* handle,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->open_memory(get_handle(), handle, out_memory, out_status.get_handle());
    }

    void close_memory(
        TF_DeviceMemoryBase* memory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->close_memory(get_handle(), memory, out_status.get_handle());
    }
};

} // namespace ice::sonic
