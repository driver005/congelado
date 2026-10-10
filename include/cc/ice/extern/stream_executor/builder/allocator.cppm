// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:allocator;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_AllocatorOps
{
public:
    explicit TF_AllocatorOps(
        const ::TF_BufferOps* TF_BufferOps_ops,
        const ::TF_MemPoolOps* TF_MemPoolOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StreamOps* TF_StreamOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_BufferOps_ops = TF_BufferOps_ops;
        m_TF_MemPoolOps_ops = TF_MemPoolOps_ops;
        m_Status_ops = Status_ops;
        m_TF_StreamOps_ops = TF_StreamOps_ops;
    }

    TF_AllocatorOps(const TF_AllocatorOps&) = delete;
    TF_AllocatorOps& operator=(const TF_AllocatorOps&) = delete;

    static TF_AllocatorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_AllocatorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_AllocatorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_AllocatorOps*>(handle->plugin_data);
    }

    virtual ~TF_AllocatorOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void allocate(
        uint64_t size,
        TF_MemorySpace memory_space,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void deallocate(TF_DeviceMemoryBase* memory) noexcept = 0;
    virtual void record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void owns_pointer(const void* pointer, _Bool* out_owns) noexcept = 0;
    virtual void get_base_allocation(
        const void* pointer,
        void** out_base,
        uint64_t* out_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void empty_cache(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_memory_fraction(double fraction, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_memory_fraction(double* out_fraction) noexcept = 0;
    virtual void set_option(
        TF_AllocatorOption option,
        int64_t value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_option(
        TF_AllocatorOption option,
        int64_t* out_value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) noexcept = 0;
    virtual void reset_accumulated_stats() noexcept = 0;
    virtual void reset_peak_stats() noexcept = 0;
    virtual void get_snapshot(
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void generate_pool_id(TF_PoolId* out_pool_id) noexcept = 0;
    virtual void create_mem_pool_internal(
        const TF_PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::TF_MemPoolOps& out_pool,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) noexcept = 0;
    virtual void
    enable_peer_access(int peer_device_index, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void export_memory(
        const TF_DeviceMemoryBase* memory,
        TF_IpcMemoryHandle* out_handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void open_memory(
        const TF_IpcMemoryHandle* handle,
        TF_DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    close_memory(TF_DeviceMemoryBase* memory, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Allocator*)) noexcept
    {
        m_vtable = ::TF_AllocatorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_AllocatorOps, close_memory),

            .create = create,
            .destroy =
                [](TF_Allocator* handle) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(handle);
                self.destroy();
            },
            .allocate =
                [](TF_Allocator* allocator,
                   uint64_t size,
                   TF_MemorySpace memory_space,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* out_memory,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.allocate(
                    size,
                    memory_space,
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    out_memory,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .deallocate =
                [](TF_Allocator* allocator, TF_DeviceMemoryBase* memory) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.deallocate(memory);
            },
            .record_stream =
                [](TF_Allocator* allocator,
                   const TF_DeviceMemoryBase* memory,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.record_stream(
                    memory,
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .owns_pointer =
                [](TF_Allocator* allocator, const void* pointer, _Bool* out_owns) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.owns_pointer(pointer, out_owns);
            },
            .get_base_allocation =
                [](TF_Allocator* allocator,
                   const void* pointer,
                   void** out_base,
                   uint64_t* out_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.get_base_allocation(
                    pointer,
                    out_base,
                    out_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .empty_cache =
                [](TF_Allocator* allocator, TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.empty_cache(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .set_memory_fraction =
                [](TF_Allocator* allocator, double fraction, TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.set_memory_fraction(
                    fraction,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_memory_fraction =
                [](TF_Allocator* allocator, double* out_fraction) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.get_memory_fraction(out_fraction);
            },
            .set_option =
                [](TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.set_option(
                    option,
                    value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_option =
                [](TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t* out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.get_option(
                    option,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stats =
                [](TF_Allocator* allocator,
                   TF_AllocatorStats* out_stats,
                   _Bool* out_success) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.get_stats(out_stats, out_success);
            },
            .reset_accumulated_stats =
                [](TF_Allocator* allocator) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.reset_accumulated_stats();
            },
            .reset_peak_stats =
                [](TF_Allocator* allocator) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.reset_peak_stats();
            },
            .get_snapshot =
                [](TF_Allocator* allocator,
                   const TF_PoolId* pool_filter,
                   TF_Buffer* out_snapshot,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.get_snapshot(
                    pool_filter,
                    self.wrap(std::type_identity<ice::sonic::TF_BufferOps>{}, out_snapshot),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .generate_pool_id =
                [](TF_Allocator* allocator, TF_PoolId* out_pool_id) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.generate_pool_id(out_pool_id);
            },
            .create_mem_pool_internal =
                [](TF_Allocator* allocator,
                   const TF_PoolId* pool_id,
                   _Bool is_user_created,
                   TF_MemPool* out_pool,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.create_mem_pool_internal(
                    pool_id,
                    is_user_created,
                    self.wrap(std::type_identity<ice::sonic::TF_MemPoolOps>{}, out_pool),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_mem_pool_internal =
                [](TF_Allocator* allocator, TF_MemPool* pool) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.destroy_mem_pool_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_MemPoolOps>{}, pool)
                );
            },
            .enable_peer_access =
                [](TF_Allocator* allocator, int peer_device_index, TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.enable_peer_access(
                    peer_device_index,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .export_memory =
                [](TF_Allocator* allocator,
                   const TF_DeviceMemoryBase* memory,
                   TF_IpcMemoryHandle* out_handle,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.export_memory(
                    memory,
                    out_handle,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .open_memory =
                [](TF_Allocator* allocator,
                   const TF_IpcMemoryHandle* handle,
                   TF_DeviceMemoryBase* out_memory,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.open_memory(
                    handle,
                    out_memory,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .close_memory =
                [](TF_Allocator* allocator,
                   TF_DeviceMemoryBase* memory,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_AllocatorOps::from_handle(allocator);
                self.close_memory(
                    memory,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_BufferOps
    wrap(std::type_identity<ice::sonic::TF_BufferOps>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::TF_BufferOps{m_TF_BufferOps_ops, const_cast<::TF_Buffer*>(handle)};
    }

    ice::sonic::TF_MemPoolOps
    wrap(std::type_identity<ice::sonic::TF_MemPoolOps>, const ::TF_MemPool* handle) const noexcept
    {
        return ice::sonic::TF_MemPoolOps{m_TF_MemPoolOps_ops, const_cast<::TF_MemPool*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::TF_StreamOps
    wrap(std::type_identity<ice::sonic::TF_StreamOps>, const ::TF_Stream* handle) const noexcept
    {
        return ice::sonic::TF_StreamOps{m_TF_StreamOps_ops, const_cast<::TF_Stream*>(handle)};
    }

    const ::TF_AllocatorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Allocator& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_AllocatorOps*>(&m_vtable)
        );
    }

private:
    ::TF_AllocatorOps m_vtable;
    ::TF_Allocator m_handle;

    const ::TF_BufferOps* m_TF_BufferOps_ops{nullptr};

    const ::TF_MemPoolOps* m_TF_MemPoolOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StreamOps* m_TF_StreamOps_ops{nullptr};
};

} // namespace ice::builder
