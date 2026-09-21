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
    static TF_AllocatorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_AllocatorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_AllocatorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_AllocatorOps*>(handle->plugin_data);
    }

    virtual ~TF_AllocatorOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> allocate(
        uint64_t size,
        TF_MemorySpace memory_space,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* out_memory
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    deallocate(TF_DeviceMemoryBase* memory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    owns_pointer(const void* pointer, _Bool* out_owns) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_base_allocation(const void* pointer, void** out_base, uint64_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> empty_cache() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_memory_fraction(double fraction) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_memory_fraction(double* out_fraction) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_option(TF_AllocatorOption option, int64_t value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_option(TF_AllocatorOption option, int64_t* out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> reset_accumulated_stats() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> reset_peak_stats() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_snapshot(
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    generate_pool_id(TF_PoolId* out_pool_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_mem_pool_internal(
        const TF_PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::TF_MemPoolOps& out_pool
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    enable_peer_access(int peer_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    export_memory(const TF_DeviceMemoryBase* memory, TF_IpcMemoryHandle* out_handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    open_memory(const TF_IpcMemoryHandle* handle, TF_DeviceMemoryBase* out_memory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    close_memory(TF_DeviceMemoryBase* memory) noexcept = 0;

    static TF_AllocatorOps* get_generic_vtable()
    {
        static TF_AllocatorOps vtable = {
            .struct_size = TF_ALLOCATOR_STRUCT_SIZE,
            .allocate =
                [](TF_Allocator* allocator,
                   uint64_t size,
                   TF_MemorySpace memory_space,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* out_memory,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->allocate(
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
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->deallocate(memory);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .record_stream =
                [](TF_Allocator* allocator,
                   const TF_DeviceMemoryBase* memory,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->record_stream(memory, ice::sonic::TF_StreamOps::wrap(stream));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .owns_pointer =
                [](TF_Allocator* allocator, const void* pointer, _Bool* out_owns) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->owns_pointer(pointer, out_owns);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_base_allocation =
                [](TF_Allocator* allocator,
                   const void* pointer,
                   void** out_base,
                   uint64_t* out_size,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->get_base_allocation(pointer, out_base, out_size);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .empty_cache =
                [](TF_Allocator* allocator, TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->empty_cache();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_memory_fraction =
                [](TF_Allocator* allocator, double fraction, TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->set_memory_fraction(fraction);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_memory_fraction =
                [](TF_Allocator* allocator, double* out_fraction) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->get_memory_fraction(out_fraction);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_option =
                [](TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->set_option(option, value);
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
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->get_option(option, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TF_Allocator* allocator,
                   TF_AllocatorStats* out_stats,
                   _Bool* out_success) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->get_stats(out_stats, out_success);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reset_accumulated_stats =
                [](TF_Allocator* allocator) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->reset_accumulated_stats();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reset_peak_stats =
                [](TF_Allocator* allocator) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->reset_peak_stats();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_snapshot =
                [](TF_Allocator* allocator,
                   const TF_PoolId* pool_filter,
                   TF_Buffer* out_snapshot,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res =
                    self->get_snapshot(pool_filter, ice::sonic::TF_BufferOps::wrap(out_snapshot));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .generate_pool_id =
                [](TF_Allocator* allocator, TF_PoolId* out_pool_id) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->generate_pool_id(out_pool_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_mem_pool_internal =
                [](TF_Allocator* allocator,
                   const TF_PoolId* pool_id,
                   _Bool is_user_created,
                   TF_MemPool* out_pool,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->create_mem_pool_internal(
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
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->destroy_mem_pool_internal(ice::sonic::TF_MemPoolOps::wrap(pool));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .enable_peer_access =
                [](TF_Allocator* allocator, int peer_device_index, TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->enable_peer_access(peer_device_index);
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
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->export_memory(memory, out_handle);
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
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->open_memory(handle, out_memory);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .close_memory =
                [](TF_Allocator* allocator,
                   TF_DeviceMemoryBase* memory,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_AllocatorOps::create(allocator);
                auto res = self->close_memory(memory);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
