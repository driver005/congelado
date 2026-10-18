// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_grappler_sonic:device_graph;

import std;
import :capture_mode;
import cc_ice_extern_random_generator_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class GrapplerDeviceGraph :
    public ice::sonic::Runtime<::TFGrapplerDeviceGraphOps, ::TFGrapplerDeviceGraph>
{
public:
    GrapplerDeviceGraph(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    GrapplerDeviceGraph(
        const ice::sonic::Registration& registry,
        const ::TFGrapplerDeviceGraph& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit GrapplerDeviceGraph(const ::TFGrapplerDeviceGraphOps* ops) noexcept :
        Runtime(ops)
    {
    }

    GrapplerDeviceGraph(
        const ::TFGrapplerDeviceGraphOps* ops,
        const ::TFGrapplerDeviceGraph* handle
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
        const ice::sonic::Stream& capture_stream,
        const ice::sonic::PoolId* pool_id,
        ice::sonic::CaptureMode mode,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->capture_begin(
            get_handle(),
            capture_stream.get_handle(),
            pool_id == nullptr ? nullptr : pool_id->get_handle(),
            mode.get_value(),
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

    void get_pool(ice::sonic::PoolId* out_pool_id) const noexcept
    {
        m_ops->get_pool(get_handle(), out_pool_id == nullptr ? nullptr : out_pool_id->get_handle());
    }

    void register_random_generator(
        const ice::sonic::RandomGenerator& generator,
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
        const ice::sonic::RandomGenerator& generator,
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
