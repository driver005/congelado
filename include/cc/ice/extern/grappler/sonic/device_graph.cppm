// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"

export module cc_ice_extern_grappler_sonic:device_graph;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGrapplerDeviceGraphOps :
    public ice::sonic::Runtime<::TFGrapplerDeviceGraphOps, ::TFGrapplerDeviceGraph>
{
public:
    template<typename Registry>
    TFGrapplerDeviceGraphOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFGrapplerDeviceGraphOps(
        Registry& registry,
        ::TFGrapplerDeviceGraph* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFGrapplerDeviceGraphOps(const ::TFGrapplerDeviceGraphOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGrapplerDeviceGraphOps(
        const ::TFGrapplerDeviceGraphOps* ops,
        ::TFGrapplerDeviceGraph* handle
    ) noexcept :
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

    void capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->capture_begin(
            get_handle(),
            capture_stream.get_handle(),
            pool_id,
            mode,
            out_status.get_handle()
        );
    }

    void capture_end(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->capture_end(get_handle(), out_status.get_handle());
    }

    void instantiate(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->instantiate(get_handle(), out_status.get_handle());
    }

    void replay(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->replay(get_handle(), out_status.get_handle());
    }

    void reset(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->reset(get_handle(), out_status.get_handle());
    }

    void get_pool(TF_PoolId* out_pool_id) const noexcept
    {
        m_ops->get_pool(get_handle(), out_pool_id);
    }

    void register_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& generator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->register_random_generator(
            get_handle(),
            generator.get_handle(),
            out_status.get_handle()
        );
    }

    void unregister_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& generator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->unregister_random_generator(
            get_handle(),
            generator.get_handle(),
            out_status.get_handle()
        );
    }

    void enable_debug_mode() const noexcept
    {
        m_ops->enable_debug_mode(get_handle());
    }

    void debug_dump(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->debug_dump(get_handle(), path.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
