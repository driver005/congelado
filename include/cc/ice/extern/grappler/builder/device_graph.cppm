// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_grappler_builder:device_graph;

import std;
import cc_ice_extern_grappler_sonic;
import cc_ice_extern_random_generator_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class GrapplerDeviceGraph
{
public:
    explicit GrapplerDeviceGraph(
        const ::TF_RandomGeneratorOps* RandomGenerator_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StreamOps* Stream_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_RandomGenerator_ops = RandomGenerator_ops;
        m_Status_ops = Status_ops;
        m_Stream_ops = Stream_ops;
        m_String_ops = String_ops;
    }

    GrapplerDeviceGraph(const GrapplerDeviceGraph&) = delete;
    GrapplerDeviceGraph& operator=(const GrapplerDeviceGraph&) = delete;

    static GrapplerDeviceGraph& from_handle(void* ctx) noexcept
    {
        return *static_cast<GrapplerDeviceGraph*>(ctx);
    }

    template<typename HandleT>
    static GrapplerDeviceGraph& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<GrapplerDeviceGraph*>(handle->plugin_data);
    }

    virtual ~GrapplerDeviceGraph() = default;
    virtual void destroy() noexcept = 0;
    virtual void capture_begin(
        const ice::sonic::Stream& capture_stream,
        const ice::sonic::PoolId* pool_id,
        ice::sonic::CaptureMode mode,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void capture_end(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void instantiate(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void replay(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void reset(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_pool(ice::sonic::PoolId* out_pool_id) noexcept = 0;
    virtual void register_random_generator(
        const ice::sonic::RandomGenerator& generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void unregister_random_generator(
        const ice::sonic::RandomGenerator& generator,
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
                [](::TFGrapplerDeviceGraph* handle) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(handle);
                self.destroy();
            },
            .capture_begin =
                [](::TFGrapplerDeviceGraph* graph,
                   ::TF_Stream* capture_stream,
                   const ::TF_PoolId* pool_id,
                   TF_CaptureMode mode,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                std::optional<ice::sonic::PoolId> wrapped_pool_id;
                if (pool_id != nullptr) {
                    wrapped_pool_id.emplace(*pool_id);
                }
                self.capture_begin(
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, capture_stream),
                    wrapped_pool_id ? &*wrapped_pool_id : nullptr,
                    ice::sonic::CaptureMode{mode},
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .capture_end =
                [](::TFGrapplerDeviceGraph* graph, ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.capture_end(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .instantiate =
                [](::TFGrapplerDeviceGraph* graph, ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.instantiate(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .replay =
                [](::TFGrapplerDeviceGraph* graph, ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.replay(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .reset =
                [](::TFGrapplerDeviceGraph* graph, ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.reset(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get_pool =
                [](::TFGrapplerDeviceGraph* graph, ::TF_PoolId* out_pool_id) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                std::optional<ice::sonic::PoolId> wrapped_out_pool_id;
                if (out_pool_id != nullptr) {
                    wrapped_out_pool_id.emplace(*out_pool_id);
                }
                self.get_pool(wrapped_out_pool_id ? &*wrapped_out_pool_id : nullptr);
                if (out_pool_id != nullptr) {
                    *out_pool_id = *wrapped_out_pool_id->get_handle();
                }
            },
            .register_random_generator =
                [](::TFGrapplerDeviceGraph* graph,
                   ::TF_RandomGenerator* generator,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.register_random_generator(
                    self.wrap(std::type_identity<ice::sonic::RandomGenerator>{}, generator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .unregister_random_generator =
                [](::TFGrapplerDeviceGraph* graph,
                   ::TF_RandomGenerator* generator,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.unregister_random_generator(
                    self.wrap(std::type_identity<ice::sonic::RandomGenerator>{}, generator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .enable_debug_mode =
                [](::TFGrapplerDeviceGraph* graph) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.enable_debug_mode();
            },
            .debug_dump =
                [](::TFGrapplerDeviceGraph* graph,
                   const ::TF_String* path,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerDeviceGraph::from_handle(graph);
                self.debug_dump(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::RandomGenerator wrap(
        std::type_identity<ice::sonic::RandomGenerator>,
        const ::TF_RandomGenerator* handle
    ) const noexcept
    {
        return ice::sonic::RandomGenerator{m_RandomGenerator_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::Stream
    wrap(std::type_identity<ice::sonic::Stream>, const ::TF_Stream* handle) const noexcept
    {
        return ice::sonic::Stream{m_Stream_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFGrapplerDeviceGraphOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerDeviceGraph& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGrapplerDeviceGraphOps*>(&m_vtable));
    }

private:
    ::TFGrapplerDeviceGraphOps m_vtable;
    ::TFGrapplerDeviceGraph m_handle;

    const ::TF_RandomGeneratorOps* m_RandomGenerator_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StreamOps* m_Stream_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
