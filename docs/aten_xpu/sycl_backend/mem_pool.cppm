// SYCL reference plugin — a named subset of SyclAllocator's blocks (for graph capture or a
// user-requested private pool).
//
// Not built by Bazel (docs/ only). Replaces core/MemPool.{h,cpp}. The pool itself holds no
// blocks — SyclAllocator looks at "is a pool currently active for this stream" when deciding
// which BlockPool to allocate from (beginAllocateToPool/endAllocateToPool in
// c10/XPUCachingAllocator.cpp); this class is just the id + refcount + filter that decision
// reads.

module;

#include "include/c/extern/memory/mem_pool.h"

export module sycl_backend:mem_pool;

import std;
import cc_ice_extern_memory_builder;

export namespace sycl_backend {

class SyclMemPool : public ice::builder::MemPool
{
public:
    SyclMemPool(TF_PoolId pool_id, bool is_user_created) noexcept :
        m_pool_id{pool_id},
        m_is_user_created{is_user_created}
    {
    }

    ~SyclMemPool() override = default;
    SyclMemPool(const SyclMemPool&) = delete;
    SyclMemPool& operator=(const SyclMemPool&) = delete;
    SyclMemPool(SyclMemPool&&) = delete;
    SyclMemPool& operator=(SyclMemPool&&) = delete;

    const TF_PoolId& get_pool_id() const noexcept
    {
        return m_pool_id;
    }

    bool is_user_created() const noexcept
    {
        return m_is_user_created;
    }

    bool matches_stream(TF_Stream* stream) const
    {
        if (!m_active) {
            return false;
        }
        if (!m_stream_filter) {
            return true;
        }

        bool matched = false;
        m_stream_filter(m_filter_data, stream, &matched);
        return matched;
    }

    void get_id(TF_PoolId* out_pool_id) noexcept override
    {
        *out_pool_id = m_pool_id;
    }

    void use_count(int* out_count) noexcept override
    {
        *out_count = m_use_count;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    begin_allocate_to_pool(TF_StreamFilterFn stream_filter, void* filter_data) noexcept override
    {
        m_stream_filter = stream_filter;
        m_filter_data = filter_data;
        m_active = true;
        ++m_use_count;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> end_allocate_to_pool() noexcept override
    {
        m_active = false;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> release() noexcept override
    {
        if (m_use_count > 0) {
            --m_use_count;
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_use_on_oom(bool use_on_oom) noexcept override
    {
        m_use_on_oom = use_on_oom;
        return {};
    }

    bool get_use_on_oom() const noexcept
    {
        return m_use_on_oom;
    }

private:
    TF_PoolId m_pool_id;
    bool m_is_user_created;
    bool m_active{false};
    bool m_use_on_oom{false};
    int m_use_count{0};
    TF_StreamFilterFn m_stream_filter{nullptr};
    void* m_filter_data{nullptr};
};

} // namespace sycl_backend
