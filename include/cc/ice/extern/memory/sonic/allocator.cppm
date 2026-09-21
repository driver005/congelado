// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/memory/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/memory/allocator.h"

export module cc_ice_extern_memory_sonic:allocator;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_AllocatorOps : public ice::sonic::Runtime<TF_AllocatorOps, TF_AllocatorOps>
{
public:
    explicit TF_AllocatorOps(TF_AllocatorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "memory";

    [[nodiscard]] std::expected<void, ice::Status> allocate(
        uint64_t size,
        TF_MemorySpace memory_space,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* out_memory
    ) noexcept
    {
        ice::Status status;
        m_ops->allocate(
            get_handle(),
            size,
            memory_space,
            stream.get_handle(),
            out_memory status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> deallocate(TF_DeviceMemoryBase* memory) noexcept
    {
        ice::Status status;
        m_ops->deallocate(get_handle(), memory status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> record_stream(
        const TF_DeviceMemoryBase* memory,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept
    {
        ice::Status status;
        m_ops->record_stream(get_handle(), memory, stream.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    owns_pointer(const void* pointer, _Bool* out_owns) noexcept
    {
        ice::Status status;
        m_ops->owns_pointer(get_handle(), pointer, out_owns status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_base_allocation(const void* pointer, void** out_base, uint64_t* out_size) noexcept
    {
        ice::Status status;
        m_ops->get_base_allocation(get_handle(), pointer, out_base, out_size status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> empty_cache() noexcept
    {
        ice::Status status;
        m_ops->empty_cache(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_memory_fraction(double fraction) noexcept
    {
        ice::Status status;
        m_ops->set_memory_fraction(get_handle(), fraction status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_memory_fraction(double* out_fraction) noexcept
    {
        ice::Status status;
        m_ops->get_memory_fraction(get_handle(), out_fraction status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_option(TF_AllocatorOption option, int64_t value) noexcept
    {
        ice::Status status;
        m_ops->set_option(get_handle(), option, value status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_option(TF_AllocatorOption option, int64_t* out_value) noexcept
    {
        ice::Status status;
        m_ops->get_option(get_handle(), option, out_value status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_stats(TF_AllocatorStats* out_stats, _Bool* out_success) noexcept
    {
        ice::Status status;
        m_ops->get_stats(get_handle(), out_stats, out_success status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> reset_accumulated_stats() noexcept
    {
        ice::Status status;
        m_ops->reset_accumulated_stats(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> reset_peak_stats() noexcept
    {
        ice::Status status;
        m_ops->reset_peak_stats(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_snapshot(
        const TF_PoolId* pool_filter,
        const ice::sonic::TF_BufferOps& out_snapshot
    ) noexcept
    {
        ice::Status status;
        m_ops->get_snapshot(
            get_handle(),
            pool_filter,
            out_snapshot.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> generate_pool_id(TF_PoolId* out_pool_id) noexcept
    {
        ice::Status status;
        m_ops->generate_pool_id(get_handle(), out_pool_id status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> create_mem_pool_internal(
        const TF_PoolId* pool_id,
        _Bool is_user_created,
        const ice::sonic::TF_MemPoolOps& out_pool
    ) noexcept
    {
        ice::Status status;
        m_ops->create_mem_pool_internal(
            get_handle(),
            pool_id,
            is_user_created,
            out_pool.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    destroy_mem_pool_internal(const ice::sonic::TF_MemPoolOps& pool) noexcept
    {
        ice::Status status;
        m_ops->destroy_mem_pool_internal(get_handle(), pool.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    enable_peer_access(int peer_device_index) noexcept
    {
        ice::Status status;
        m_ops->enable_peer_access(get_handle(), peer_device_index status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    export_memory(const TF_DeviceMemoryBase* memory, TF_IpcMemoryHandle* out_handle) noexcept
    {
        ice::Status status;
        m_ops->export_memory(get_handle(), memory, out_handle status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    open_memory(const TF_IpcMemoryHandle* handle, TF_DeviceMemoryBase* out_memory) noexcept
    {
        ice::Status status;
        m_ops->open_memory(get_handle(), handle, out_memory status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    close_memory(TF_DeviceMemoryBase* memory) noexcept
    {
        ice::Status status;
        m_ops->close_memory(get_handle(), memory status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
