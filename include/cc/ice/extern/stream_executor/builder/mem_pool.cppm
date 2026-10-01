// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/mem_pool.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:mem_pool;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_MemPoolOps
{
public:
    explicit TF_MemPoolOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_id(TF_PoolId* out_pool_id) noexcept = 0;
    virtual void use_count(int* out_count) noexcept = 0;
    virtual void begin_allocate_to_pool(
        TF_StreamFilterFn stream_filter,
        void* filter_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void end_allocate_to_pool(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void release(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_use_on_oom(_Bool use_on_oom, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_MemPool*)) noexcept
    {
        m_vtable = ::TF_MemPoolOps{
            .struct_size = TF_OFFSET_OF_END(::TF_MemPoolOps, set_use_on_oom),

            .create = create,
            .destroy =
                [](TF_MemPool* handle) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(handle);
                self.destroy();
            },
            .get_id =
                [](TF_MemPool* pool, TF_PoolId* out_pool_id) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(pool);
                self.get_id(out_pool_id);
            },
            .use_count =
                [](TF_MemPool* pool, int* out_count) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(pool);
                self.use_count(out_count);
            },
            .begin_allocate_to_pool =
                [](TF_MemPool* pool,
                   TF_StreamFilterFn stream_filter,
                   void* filter_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(pool);
                self.begin_allocate_to_pool(
                    stream_filter,
                    filter_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .end_allocate_to_pool =
                [](TF_MemPool* pool, TF_Status* out_status) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(pool);
                self.end_allocate_to_pool(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .release =
                [](TF_MemPool* pool, TF_Status* out_status) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(pool);
                self.release(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .set_use_on_oom =
                [](TF_MemPool* pool, _Bool use_on_oom, TF_Status* out_status) noexcept
            {
                auto& self = TF_MemPoolOps::from_handle(pool);
                self.set_use_on_oom(
                    use_on_oom,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_MemPoolOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_MemPool& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_MemPoolOps*>(&m_vtable));
    }

private:
    ::TF_MemPoolOps m_vtable;
    ::TF_MemPool m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
