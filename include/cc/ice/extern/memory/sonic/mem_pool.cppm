// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/memory/mem_pool.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/memory/mem_pool.h"

export module cc_ice_extern_memory_sonic:mem_pool;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_MemPoolOps : public ice::sonic::Runtime<TF_MemPoolOps, TF_MemPoolOps>
{
public:
    explicit TF_MemPoolOps(TF_MemPoolOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "memory";

    [[nodiscard]] std::expected<void, ice::Status> get_id(TF_PoolId* out_pool_id) noexcept
    {
        ice::Status status;
        m_ops->get_id(get_handle(), out_pool_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> use_count(int* out_count) noexcept
    {
        ice::Status status;
        m_ops->use_count(get_handle(), out_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    begin_allocate_to_pool(TF_StreamFilterFn stream_filter, void* filter_data) noexcept
    {
        ice::Status status;
        m_ops
            ->begin_allocate_to_pool(get_handle(), stream_filter, filter_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> end_allocate_to_pool() noexcept
    {
        ice::Status status;
        m_ops->end_allocate_to_pool(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> release() noexcept
    {
        ice::Status status;
        m_ops->release(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_use_on_oom(_Bool use_on_oom) noexcept
    {
        ice::Status status;
        m_ops->set_use_on_oom(get_handle(), use_on_oom, status.get_handle());

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
