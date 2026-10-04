module;

#include "include/c/extern/stream_executor/mem_pool.h"

export module aten_xpu_extern_stream_executor:mem_pool;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;
import :allocator_block_pool;

export namespace aten_xpu {

class SyclMemPool : public ice::builder::TF_MemPoolOps
{
public:
    using RawAllocate = std::function<void*(std::size_t)>;
    using RawDeallocate = std::function<void(void*)>;

    explicit SyclMemPool(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_MemPoolOps{ops.getStatusOps()},
        m_status{ops}
    {
    }

    ~SyclMemPool() override = default;
    SyclMemPool(const SyclMemPool&) = delete;
    SyclMemPool& operator=(const SyclMemPool&) = delete;
    SyclMemPool(SyclMemPool&&) = delete;
    SyclMemPool& operator=(SyclMemPool&&) = delete;

    static void create(::TF_MemPool* handle)
    {

        auto* pool = new SyclMemPool{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *pool);

    }

    void setNoSplit(bool no_split) noexcept { m_no_split = no_split; }

    void setRawAllocator(RawAllocate allocate, RawDeallocate deallocate)
    {

        m_raw_allocate = std::move(allocate);
        m_raw_deallocate = std::move(deallocate);

    }

    void bind(TF_PoolId pool_id, bool is_user_created)
    {

        m_pool_id = pool_id;
        m_is_user_created = is_user_created;
        m_use_count = 1;
        m_small_blocks = std::make_unique<SyclBlockPool>(true, pool_id);
        m_large_blocks = std::make_unique<SyclBlockPool>(false, pool_id);

    }

    void increment_use() noexcept { ++m_use_count; }

    bool matches(::TF_Stream* stream) const
    {

        if (!m_allocating) {
            return false;
        }
        if (m_stream_filter == nullptr) {
            return true;
        }
        _Bool matched = false;
        m_stream_filter(m_filter_data, stream, &matched);
        return matched;

    }

    bool is_freeable() const noexcept
    {

        return m_use_count == 0 && m_small_blocks->getAllocationCount() == 0 &&
               m_large_blocks->getAllocationCount() == 0;

    }

    void destroy() noexcept override { delete this; }

    void get_id(TF_PoolId* out_pool_id) noexcept override { *out_pool_id = m_pool_id; }

    void use_count(int* out_count) noexcept override { *out_count = m_use_count; }

    void begin_allocate_to_pool(
        TF_StreamFilterFn stream_filter,
        void* filter_data,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (m_allocating) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "pool is already capturing");
            return;
        }
        m_stream_filter = stream_filter;
        m_filter_data = filter_data;
        m_allocating = true;

    }

    void end_allocate_to_pool(const ice::sonic::Status& out_status) noexcept override
    {

        static_cast<void>(out_status);
        m_allocating = false;
        m_stream_filter = nullptr;
        m_filter_data = nullptr;

    }

    void release(const ice::sonic::Status& out_status) noexcept override
    {

        if (m_use_count <= 0) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "pool released more than acquired");
            return;
        }
        --m_use_count;

    }

    void set_use_on_oom(_Bool use_on_oom, const ice::sonic::Status& out_status) noexcept override
    {

        static_cast<void>(out_status);
        m_use_on_oom = use_on_oom;

    }

    const TF_PoolId& getPoolId() const noexcept { return m_pool_id; }

    bool getIsUserCreated() const noexcept { return m_is_user_created; }

    bool getUseOnOom() const noexcept { return m_use_on_oom; }

    bool getNoSplit() const noexcept { return m_no_split; }

    int getUseCount() const noexcept { return m_use_count; }

    SyclBlockPool& getSmallBlocks() noexcept { return *m_small_blocks; }

    SyclBlockPool& getLargeBlocks() noexcept { return *m_large_blocks; }

    const RawAllocate& getRawAllocate() const noexcept { return m_raw_allocate; }

    const RawDeallocate& getRawDeallocate() const noexcept { return m_raw_deallocate; }

private:
    SyclStatus m_status;
    TF_PoolId m_pool_id{};
    bool m_is_user_created{true};
    bool m_allocating{false};
    bool m_use_on_oom{false};
    bool m_no_split{false};
    int m_use_count{0};
    TF_StreamFilterFn m_stream_filter{nullptr};
    void* m_filter_data{nullptr};
    std::unique_ptr<SyclBlockPool> m_small_blocks;
    std::unique_ptr<SyclBlockPool> m_large_blocks;
    RawAllocate m_raw_allocate;
    RawDeallocate m_raw_deallocate;
};

} // namespace aten_xpu
