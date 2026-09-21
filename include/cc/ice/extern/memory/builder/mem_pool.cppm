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
    static TF_MemPoolOps* create(void* ctx) noexcept
    {
        return static_cast<TF_MemPoolOps*>(ctx);
    }

    template<typename HandleT>
    static TF_MemPoolOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_MemPoolOps*>(handle->plugin_data);
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

    static TF_MemPoolOps* get_generic_vtable()
    {
        static TF_MemPoolOps vtable = {
            .struct_size = TF_MEMPOOL_STRUCT_SIZE,
            .get_id =
                [](TF_MemPool* pool, TF_PoolId* out_pool_id) noexcept
            {
                auto* self = TF_MemPoolOps::create(pool);
                auto res = self->get_id(out_pool_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .use_count =
                [](TF_MemPool* pool, int* out_count) noexcept
            {
                auto* self = TF_MemPoolOps::create(pool);
                auto res = self->use_count(out_count);
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
                auto* self = TF_MemPoolOps::create(pool);
                auto res = self->begin_allocate_to_pool(stream_filter, filter_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .end_allocate_to_pool =
                [](TF_MemPool* pool, TF_Status* out_status) noexcept
            {
                auto* self = TF_MemPoolOps::create(pool);
                auto res = self->end_allocate_to_pool();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .release =
                [](TF_MemPool* pool, TF_Status* out_status) noexcept
            {
                auto* self = TF_MemPoolOps::create(pool);
                auto res = self->release();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_use_on_oom =
                [](TF_MemPool* pool, _Bool use_on_oom, TF_Status* out_status) noexcept
            {
                auto* self = TF_MemPoolOps::create(pool);
                auto res = self->set_use_on_oom(use_on_oom);
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
