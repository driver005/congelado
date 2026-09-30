// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/memory/mem_pool.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/memory/mem_pool.h"

export module cc_ice_extern_memory_builder:mem_pool;

import std;

export namespace ice::builder {

class TF_MemPoolOps
{
public:
    TF_MemPoolOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_MemPoolOps(const TF_MemPoolOps&) = delete;
    TF_MemPoolOps& operator=(const TF_MemPoolOps&) = delete;

    static TF_MemPoolOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_MemPoolOps*>(ctx);
    }

    template<typename HandleT>
    static TF_MemPoolOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_MemPoolOps*>(handle->plugin_data);
    }

    virtual ~TF_MemPoolOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_id(TF_PoolId* out_pool_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> use_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    begin_allocate_to_pool(TF_StreamFilterFn stream_filter, void* filter_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> end_allocate_to_pool() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> release() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_use_on_oom(_Bool use_on_oom) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_MemPoolOps{
            .struct_size = TF_MEMPOOL_STRUCT_SIZE,
            .get_id =
                [](TF_MemPool* pool, TF_PoolId* out_pool_id) noexcept
            {
                auto res = TF_MemPoolOps::from_handle(pool).get_id(out_pool_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .use_count =
                [](TF_MemPool* pool, int* out_count) noexcept
            {
                auto res = TF_MemPoolOps::from_handle(pool).use_count(out_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .begin_allocate_to_pool =
                [](TF_MemPool* pool,
                   TF_StreamFilterFn stream_filter,
                   void* filter_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_MemPoolOps::from_handle(pool).begin_allocate_to_pool(
                    stream_filter,
                    filter_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .end_allocate_to_pool =
                [](TF_MemPool* pool, TF_Status* out_status) noexcept
            {
                auto res = TF_MemPoolOps::from_handle(pool).end_allocate_to_pool();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .release =
                [](TF_MemPool* pool, TF_Status* out_status) noexcept
            {
                auto res = TF_MemPoolOps::from_handle(pool).release();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_use_on_oom =
                [](TF_MemPool* pool, _Bool use_on_oom, TF_Status* out_status) noexcept
            {
                auto res = TF_MemPoolOps::from_handle(pool).set_use_on_oom(use_on_oom);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_MemPoolOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_MemPool& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_MemPoolOps m_vtable;
    TF_MemPool m_handle;
};

} // namespace ice::builder
