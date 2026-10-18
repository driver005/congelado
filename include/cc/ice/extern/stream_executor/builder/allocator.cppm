// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:allocator;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Allocator
{
public:
    explicit Allocator(
        const ::TF_BufferOps* Buffer_ops,
        const ::TF_MemPoolOps* MemPool_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StreamOps* Stream_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Buffer_ops = Buffer_ops;
        m_MemPool_ops = MemPool_ops;
        m_Status_ops = Status_ops;
        m_Stream_ops = Stream_ops;
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    static Allocator& from_handle(void* ctx) noexcept
    {
        return *static_cast<Allocator*>(ctx);
    }

    template<typename HandleT>
    static Allocator& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Allocator*>(handle->plugin_data);
    }

    virtual ~Allocator() = default;
    virtual void destroy() noexcept = 0;
    virtual void allocate(
        uint64_t size,
        ice::sonic::MemorySpace memory_space,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void deallocate(ice::sonic::DeviceMemoryBase* memory) noexcept = 0;
    virtual void record_stream(
        const ice::sonic::DeviceMemoryBase* memory,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void owns_pointer(const void* pointer, bool& out_owns) noexcept = 0;
    virtual void get_base_allocation(
        const void* pointer,
        void** out_base,
        std::uint64_t& out_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void empty_cache(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_memory_fraction(double fraction, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_memory_fraction(double& out_fraction) noexcept = 0;
    virtual void set_option(
        ice::sonic::AllocatorOption option,
        int64_t value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_option(
        ice::sonic::AllocatorOption option,
        std::int64_t& out_value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stats(ice::sonic::AllocatorStats* out_stats, bool& out_success) noexcept = 0;
    virtual void reset_accumulated_stats() noexcept = 0;
    virtual void reset_peak_stats() noexcept = 0;
    virtual void get_snapshot(
        const ice::sonic::PoolId* pool_filter,
        const ice::sonic::Buffer& out_snapshot,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void generate_pool_id(ice::sonic::PoolId* out_pool_id) noexcept = 0;
    virtual void create_mem_pool_internal(
        const ice::sonic::PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::MemPool& out_pool,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_mem_pool_internal(const ice::sonic::MemPool& pool) noexcept = 0;
    virtual void
    enable_peer_access(int peer_device_index, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void export_memory(
        const ice::sonic::DeviceMemoryBase* memory,
        ice::sonic::IpcMemoryHandle* out_handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void open_memory(
        const ice::sonic::IpcMemoryHandle* handle,
        ice::sonic::DeviceMemoryBase* out_memory,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void close_memory(
        ice::sonic::DeviceMemoryBase* memory,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Allocator*)) noexcept
    {
        m_vtable = ::TF_AllocatorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_AllocatorOps, close_memory),

            .create = create,
            .destroy =
                [](::TF_Allocator* handle) noexcept
            {
                auto& self = Allocator::from_handle(handle);
                self.destroy();
            },
            .allocate =
                [](::TF_Allocator* allocator,
                   uint64_t size,
                   TF_MemorySpace memory_space,
                   ::TF_Stream* stream,
                   ::TF_DeviceMemoryBase* out_memory,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_out_memory;
                if (out_memory != nullptr) {
                    wrapped_out_memory.emplace(*out_memory);
                }
                self.allocate(
                    size,
                    ice::sonic::MemorySpace{memory_space},
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    wrapped_out_memory ? &*wrapped_out_memory : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (out_memory != nullptr) {
                    *out_memory = *wrapped_out_memory->get_handle();
                }
            },
            .deallocate =
                [](::TF_Allocator* allocator, ::TF_DeviceMemoryBase* memory) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_memory;
                if (memory != nullptr) {
                    wrapped_memory.emplace(*memory);
                }
                self.deallocate(wrapped_memory ? &*wrapped_memory : nullptr);
                if (memory != nullptr) {
                    *memory = *wrapped_memory->get_handle();
                }
            },
            .record_stream =
                [](::TF_Allocator* allocator,
                   const ::TF_DeviceMemoryBase* memory,
                   ::TF_Stream* stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_memory;
                if (memory != nullptr) {
                    wrapped_memory.emplace(*memory);
                }
                self.record_stream(
                    wrapped_memory ? &*wrapped_memory : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .owns_pointer =
                [](::TF_Allocator* allocator, const void* pointer, _Bool* out_owns) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.owns_pointer(pointer, *out_owns);
            },
            .get_base_allocation =
                [](::TF_Allocator* allocator,
                   const void* pointer,
                   void** out_base,
                   uint64_t* out_size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.get_base_allocation(
                    pointer,
                    out_base,
                    *out_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .empty_cache =
                [](::TF_Allocator* allocator, ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.empty_cache(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .set_memory_fraction =
                [](::TF_Allocator* allocator, double fraction, ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.set_memory_fraction(
                    fraction,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_memory_fraction =
                [](::TF_Allocator* allocator, double* out_fraction) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.get_memory_fraction(*out_fraction);
            },
            .set_option =
                [](::TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t value,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.set_option(
                    ice::sonic::AllocatorOption{option},
                    value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_option =
                [](::TF_Allocator* allocator,
                   TF_AllocatorOption option,
                   int64_t* out_value,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.get_option(
                    ice::sonic::AllocatorOption{option},
                    *out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stats =
                [](::TF_Allocator* allocator,
                   ::TF_AllocatorStats* out_stats,
                   _Bool* out_success) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::AllocatorStats> wrapped_out_stats;
                if (out_stats != nullptr) {
                    wrapped_out_stats.emplace(*out_stats);
                }
                self.get_stats(wrapped_out_stats ? &*wrapped_out_stats : nullptr, *out_success);
                if (out_stats != nullptr) {
                    *out_stats = *wrapped_out_stats->get_handle();
                }
            },
            .reset_accumulated_stats =
                [](::TF_Allocator* allocator) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.reset_accumulated_stats();
            },
            .reset_peak_stats =
                [](::TF_Allocator* allocator) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.reset_peak_stats();
            },
            .get_snapshot =
                [](::TF_Allocator* allocator,
                   const ::TF_PoolId* pool_filter,
                   ::TF_Buffer* out_snapshot,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::PoolId> wrapped_pool_filter;
                if (pool_filter != nullptr) {
                    wrapped_pool_filter.emplace(*pool_filter);
                }
                self.get_snapshot(
                    wrapped_pool_filter ? &*wrapped_pool_filter : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Buffer>{}, out_snapshot),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .generate_pool_id =
                [](::TF_Allocator* allocator, ::TF_PoolId* out_pool_id) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::PoolId> wrapped_out_pool_id;
                if (out_pool_id != nullptr) {
                    wrapped_out_pool_id.emplace(*out_pool_id);
                }
                self.generate_pool_id(wrapped_out_pool_id ? &*wrapped_out_pool_id : nullptr);
                if (out_pool_id != nullptr) {
                    *out_pool_id = *wrapped_out_pool_id->get_handle();
                }
            },
            .create_mem_pool_internal =
                [](::TF_Allocator* allocator,
                   const ::TF_PoolId* pool_id,
                   _Bool is_user_created,
                   ::TF_MemPool* out_pool,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::PoolId> wrapped_pool_id;
                if (pool_id != nullptr) {
                    wrapped_pool_id.emplace(*pool_id);
                }
                self.create_mem_pool_internal(
                    wrapped_pool_id ? &*wrapped_pool_id : nullptr,
                    is_user_created,
                    self.wrap(std::type_identity<ice::sonic::MemPool>{}, out_pool),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_mem_pool_internal =
                [](::TF_Allocator* allocator, ::TF_MemPool* pool) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.destroy_mem_pool_internal(
                    self.wrap(std::type_identity<ice::sonic::MemPool>{}, pool)
                );
            },
            .enable_peer_access =
                [](::TF_Allocator* allocator,
                   int peer_device_index,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                self.enable_peer_access(
                    peer_device_index,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .export_memory =
                [](::TF_Allocator* allocator,
                   const ::TF_DeviceMemoryBase* memory,
                   ::TF_IpcMemoryHandle* out_handle,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_memory;
                if (memory != nullptr) {
                    wrapped_memory.emplace(*memory);
                }
                std::optional<ice::sonic::IpcMemoryHandle> wrapped_out_handle;
                if (out_handle != nullptr) {
                    wrapped_out_handle.emplace(*out_handle);
                }
                self.export_memory(
                    wrapped_memory ? &*wrapped_memory : nullptr,
                    wrapped_out_handle ? &*wrapped_out_handle : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (out_handle != nullptr) {
                    *out_handle = *wrapped_out_handle->get_handle();
                }
            },
            .open_memory =
                [](::TF_Allocator* allocator,
                   const ::TF_IpcMemoryHandle* handle,
                   ::TF_DeviceMemoryBase* out_memory,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::IpcMemoryHandle> wrapped_handle;
                if (handle != nullptr) {
                    wrapped_handle.emplace(*handle);
                }
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_out_memory;
                if (out_memory != nullptr) {
                    wrapped_out_memory.emplace(*out_memory);
                }
                self.open_memory(
                    wrapped_handle ? &*wrapped_handle : nullptr,
                    wrapped_out_memory ? &*wrapped_out_memory : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (out_memory != nullptr) {
                    *out_memory = *wrapped_out_memory->get_handle();
                }
            },
            .close_memory =
                [](::TF_Allocator* allocator,
                   ::TF_DeviceMemoryBase* memory,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Allocator::from_handle(allocator);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_memory;
                if (memory != nullptr) {
                    wrapped_memory.emplace(*memory);
                }
                self.close_memory(
                    wrapped_memory ? &*wrapped_memory : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (memory != nullptr) {
                    *memory = *wrapped_memory->get_handle();
                }
            },

        };
    }

    ice::sonic::Buffer
    wrap(std::type_identity<ice::sonic::Buffer>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::Buffer{m_Buffer_ops, handle};
    }

    ice::sonic::MemPool
    wrap(std::type_identity<ice::sonic::MemPool>, const ::TF_MemPool* handle) const noexcept
    {
        return ice::sonic::MemPool{m_MemPool_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::Stream
    wrap(std::type_identity<ice::sonic::Stream>, const ::TF_Stream* handle) const noexcept
    {
        return ice::sonic::Stream{m_Stream_ops, handle};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_AllocatorOps*>(&m_vtable));
    }

private:
    ::TF_AllocatorOps m_vtable;
    ::TF_Allocator m_handle;

    const ::TF_BufferOps* m_Buffer_ops{nullptr};

    const ::TF_MemPoolOps* m_MemPool_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StreamOps* m_Stream_ops{nullptr};
};

} // namespace ice::builder
