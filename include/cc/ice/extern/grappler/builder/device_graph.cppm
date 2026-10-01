// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_grappler_builder:device_graph;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGrapplerDeviceGraphOps
{
public:
    explicit TFGrapplerDeviceGraphOps(
        const ::TF_RandomGeneratorOps* TF_RandomGeneratorOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StreamOps* TF_StreamOps_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_RandomGeneratorOps_ops = TF_RandomGeneratorOps_ops;
        m_Status_ops = Status_ops;
        m_TF_StreamOps_ops = TF_StreamOps_ops;
        m_String_ops = String_ops;
    }

    TFGrapplerDeviceGraphOps(const TFGrapplerDeviceGraphOps&) = delete;
    TFGrapplerDeviceGraphOps& operator=(const TFGrapplerDeviceGraphOps&) = delete;

    static TFGrapplerDeviceGraphOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerDeviceGraphOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerDeviceGraphOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerDeviceGraphOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerDeviceGraphOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void capture_end(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void instantiate(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void replay(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void reset(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_pool(TF_PoolId* out_pool_id) noexcept = 0;
    virtual void register_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void unregister_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void enable_debug_mode() noexcept = 0;
    virtual void
    debug_dump(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerDeviceGraph*)) noexcept
    {
        m_vtable = ::TFGrapplerDeviceGraphOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerDeviceGraphOps, debug_dump),

            .create = create,
            .destroy =
                [](TFGrapplerDeviceGraph* handle) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(handle);
                self.destroy();
            },
            .capture_begin =
                [](TFGrapplerDeviceGraph* graph,
                   TF_Stream* capture_stream,
                   const TF_PoolId* pool_id,
                   TF_CaptureMode mode,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.capture_begin(
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, capture_stream),
                    pool_id,
                    mode,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .capture_end =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.capture_end(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .instantiate =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.instantiate(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .replay =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.replay(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .reset =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.reset(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get_pool =
                [](TFGrapplerDeviceGraph* graph, TF_PoolId* out_pool_id) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.get_pool(out_pool_id);
            },
            .register_random_generator =
                [](TFGrapplerDeviceGraph* graph,
                   TF_RandomGenerator* generator,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.register_random_generator(
                    self.wrap(std::type_identity<ice::sonic::TF_RandomGeneratorOps>{}, generator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .unregister_random_generator =
                [](TFGrapplerDeviceGraph* graph,
                   TF_RandomGenerator* generator,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.unregister_random_generator(
                    self.wrap(std::type_identity<ice::sonic::TF_RandomGeneratorOps>{}, generator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .enable_debug_mode =
                [](TFGrapplerDeviceGraph* graph) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.enable_debug_mode();
            },
            .debug_dump =
                [](TFGrapplerDeviceGraph* graph,
                   const TF_String* path,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerDeviceGraphOps::from_handle(graph);
                self.debug_dump(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_RandomGeneratorOps wrap(
        std::type_identity<ice::sonic::TF_RandomGeneratorOps>,
        const ::TF_RandomGenerator* handle
    ) const noexcept
    {
        return ice::sonic::TF_RandomGeneratorOps{
            m_TF_RandomGeneratorOps_ops,
            const_cast<::TF_RandomGenerator*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::TF_StreamOps
    wrap(std::type_identity<ice::sonic::TF_StreamOps>, const ::TF_Stream* handle) const noexcept
    {
        return ice::sonic::TF_StreamOps{m_TF_StreamOps_ops, const_cast<::TF_Stream*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFGrapplerDeviceGraphOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerDeviceGraph& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFGrapplerDeviceGraphOps*>(&m_vtable));
    }

private:
    ::TFGrapplerDeviceGraphOps m_vtable;
    ::TFGrapplerDeviceGraph m_handle;

    const ::TF_RandomGeneratorOps* m_TF_RandomGeneratorOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StreamOps* m_TF_StreamOps_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
