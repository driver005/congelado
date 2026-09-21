// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"

export module cc_ice_extern_grappler_builder:device_graph;

import std;

export namespace ice::builder {

class TFGrapplerDeviceGraphOps
{
public:
    static TFGrapplerDeviceGraphOps* create(void* ctx) noexcept
    {
        return static_cast<TFGrapplerDeviceGraphOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerDeviceGraphOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGrapplerDeviceGraphOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerDeviceGraphOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> capture_end() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> instantiate() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> replay() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> reset() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_pool(TF_PoolId* out_pool_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    register_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    unregister_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> enable_debug_mode() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    debug_dump(const char* path) noexcept = 0;

    static TFGrapplerDeviceGraphOps* get_generic_vtable()
    {
        static TFGrapplerDeviceGraphOps vtable = {
            .struct_size = TF_RAPPLERDEVICEGRAPH_STRUCT_SIZE,
            .capture_begin =
                [](TFGrapplerDeviceGraph* graph,
                   TF_Stream* capture_stream,
                   const TF_PoolId* pool_id,
                   TF_CaptureMode mode,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->capture_begin(
                    ice::sonic::TF_StreamOps::wrap(capture_stream),
                    pool_id,
                    mode
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .capture_end =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->capture_end();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .instantiate =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->instantiate();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .replay =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->replay();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .reset =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->reset();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_pool =
                [](TFGrapplerDeviceGraph* graph, TF_PoolId* out_pool_id) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->get_pool(out_pool_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .register_random_generator =
                [](TFGrapplerDeviceGraph* graph,
                   TF_RandomGenerator* generator,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->register_random_generator(
                    ice::sonic::TF_RandomGeneratorOps::wrap(generator)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .unregister_random_generator =
                [](TFGrapplerDeviceGraph* graph,
                   TF_RandomGenerator* generator,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->unregister_random_generator(
                    ice::sonic::TF_RandomGeneratorOps::wrap(generator)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .enable_debug_mode =
                [](TFGrapplerDeviceGraph* graph) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->enable_debug_mode();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .debug_dump =
                [](TFGrapplerDeviceGraph* graph, const char* path, TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerDeviceGraphOps::create(graph);
                auto res = self->debug_dump(path);
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
