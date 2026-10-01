// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/mem_pool.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/mem_pool.h"

export module cc_ice_extern_stream_executor_sonic:mem_pool;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_MemPoolOps : public ice::sonic::Runtime<::TF_MemPoolOps, ::TF_MemPool>
{
public:
    template<typename Registry>
    TF_MemPoolOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_MemPoolOps(
        Registry& registry,
        ::TF_MemPool* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_MemPoolOps(const ::TF_MemPoolOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_MemPoolOps(const ::TF_MemPoolOps* ops, ::TF_MemPool* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_id(TF_PoolId* out_pool_id) const noexcept
    {
        m_ops->get_id(get_handle(), out_pool_id);
    }

    void use_count(int* out_count) const noexcept
    {
        m_ops->use_count(get_handle(), out_count);
    }

    void begin_allocate_to_pool(
        TF_StreamFilterFn stream_filter,
        void* filter_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->begin_allocate_to_pool(
            get_handle(),
            stream_filter,
            filter_data,
            out_status.get_handle()
        );
    }

    void end_allocate_to_pool(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->end_allocate_to_pool(get_handle(), out_status.get_handle());
    }

    void release(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->release(get_handle(), out_status.get_handle());
    }

    void set_use_on_oom(_Bool use_on_oom, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_use_on_oom(get_handle(), use_on_oom, out_status.get_handle());
    }
};

} // namespace ice::sonic
