// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/memory/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/memory/allocator.h"

export module cc_ice_extern_memory_builder:allocator;

import std;

export namespace ice::builder {

class TF_AllocatorOps
{
public:
    TF_AllocatorOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> allocate(
        uint64_t size,
        TF_MemorySpace memory_space,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* out_memory
    ) noexcept = 0;
    virtual void deallocate(TF_DeviceMemoryBase* memory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    virtual void owns_pointer(const void* pointer, _Bool* out_owns) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_base_allocation(const void* pointer, void** out_base, uint64_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> empty_cache() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_memory_fraction(double fraction) noexcept = 0;
    virtual void get_memory_fraction(double* out_fraction) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_option(TF_AllocatorOption option, int64_t value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_option(TF_AllocatorOption option, int64_t* out_value) noexcept = 0;
    virtual void get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) noexcept = 0;
    virtual void reset_accumulated_stats() noexcept = 0;
    virtual void reset_peak_stats() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_snapshot(
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot
    ) noexcept = 0;
    virtual void generate_pool_id(TF_PoolId* out_pool_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> create_mem_pool_internal(
        const TF_PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::TF_MemPoolOps& out_pool
    ) noexcept = 0;
    virtual void destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    enable_peer_access(int peer_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    export_memory(const TF_DeviceMemoryBase* memory, TF_IpcMemoryHandle* out_handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    open_memory(const TF_IpcMemoryHandle* handle, TF_DeviceMemoryBase* out_memory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    close_memory(TF_DeviceMemoryBase* memory) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_AllocatorOps{
            .struct_size = TF_ALLOCATOR_STRUCT_SIZE,
            .allocate =
                [](TF_Allocator* allocator,
                   uint64_t size,
                   TF_MemorySpace memory_space,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* out_memory,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).allocate(
                    size,
                    memory_space,
                    ice::sonic::TF_StreamOps::wrap(stream),
                    out_memory
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .deallocate =
                [](TF_Allocator* allocator, TF_DeviceMemoryBase* memory) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).deallocate(memory);
            },
            .record_stream =
                [](TF_Allocator* allocator,
                   const TF_DeviceMemoryBase* memory,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).record_stream(
                    memory,
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .owns_pointer =
                [](TF_Allocator* allocator, const void* pointer, _Bool* out_owns) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).owns_pointer(pointer, out_owns);
            },
            .get_base_allocation =
                [](TF_Allocator* allocator,
                   const void* pointer,
                   void** out_base,
                   uint64_t* out_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator)
                               .get_base_allocation(pointer, out_base, out_size);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .empty_cache =
                [](TF_Allocator* allocator, TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).empty_cache();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_memory_fraction =
                [](TF_Allocator* allocator, double fraction, TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).set_memory_fraction(fraction);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_memory_fraction =
                [](TF_Allocator* allocator, double* out_fraction) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).get_memory_fraction(out_fraction);
            },
            .set_option =
                [](TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).set_option(option, value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_option =
                [](TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t* out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).get_option(option, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TF_Allocator* allocator,
                   TF_AllocatorStats* out_stats,
                   _Bool* out_success) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).get_stats(out_stats, out_success);
            },
            .reset_accumulated_stats =
                [](TF_Allocator* allocator) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).reset_accumulated_stats();
            },
            .reset_peak_stats =
                [](TF_Allocator* allocator) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).reset_peak_stats();
            },
            .get_snapshot =
                [](TF_Allocator* allocator,
                   const TF_PoolId* pool_filter,
                   TF_Buffer* out_snapshot,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).get_snapshot(
                    pool_filter,
                    ice::sonic::TF_BufferOps::wrap(out_snapshot)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .generate_pool_id =
                [](TF_Allocator* allocator, TF_PoolId* out_pool_id) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).generate_pool_id(out_pool_id);
            },
            .create_mem_pool_internal =
                [](TF_Allocator* allocator,
                   const TF_PoolId* pool_id,
                   _Bool is_user_created,
                   TF_MemPool* out_pool,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).create_mem_pool_internal(
                    pool_id,
                    is_user_created,
                    ice::sonic::TF_MemPoolOps::wrap(out_pool)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_mem_pool_internal =
                [](TF_Allocator* allocator, TF_MemPool* pool) noexcept
            {
                TF_AllocatorOps::from_handle(allocator).destroy_mem_pool_internal(
                    ice::sonic::TF_MemPoolOps::wrap(pool)
                );
            },
            .enable_peer_access =
                [](TF_Allocator* allocator, int peer_device_index, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_AllocatorOps::from_handle(allocator).enable_peer_access(peer_device_index);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .export_memory =
                [](TF_Allocator* allocator,
                   const TF_DeviceMemoryBase* memory,
                   TF_IpcMemoryHandle* out_handle,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TF_AllocatorOps::from_handle(allocator).export_memory(memory, out_handle);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .open_memory =
                [](TF_Allocator* allocator,
                   const TF_IpcMemoryHandle* handle,
                   TF_DeviceMemoryBase* out_memory,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).open_memory(handle, out_memory);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .close_memory =
                [](TF_Allocator* allocator,
                   TF_DeviceMemoryBase* memory,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_AllocatorOps::from_handle(allocator).close_memory(memory);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_AllocatorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Allocator& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_AllocatorOps m_vtable;
    TF_Allocator m_handle;
};

} // namespace ice::builder
