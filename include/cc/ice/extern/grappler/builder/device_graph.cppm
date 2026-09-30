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
    TFGrapplerDeviceGraphOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> capture_end() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> instantiate() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> replay() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> reset() noexcept = 0;
    virtual void get_pool(TF_PoolId* out_pool_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    register_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    unregister_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept = 0;
    virtual void enable_debug_mode() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    debug_dump(const ice::sonic::String& path) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGrapplerDeviceGraphOps{
            .struct_size = TF_RAPPLERDEVICEGRAPH_STRUCT_SIZE,
            .capture_begin =
                [](TFGrapplerDeviceGraph* graph,
                   TF_Stream* capture_stream,
                   const TF_PoolId* pool_id,
                   TF_CaptureMode mode,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).capture_begin(
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
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).capture_end();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .instantiate =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).instantiate();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .replay =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).replay();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .reset =
                [](TFGrapplerDeviceGraph* graph, TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).reset();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_pool =
                [](TFGrapplerDeviceGraph* graph, TF_PoolId* out_pool_id) noexcept
            {
                TFGrapplerDeviceGraphOps::from_handle(graph).get_pool(out_pool_id);
            },
            .register_random_generator =
                [](TFGrapplerDeviceGraph* graph,
                   TF_RandomGenerator* generator,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).register_random_generator(
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
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).unregister_random_generator(
                    ice::sonic::TF_RandomGeneratorOps::wrap(generator)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .enable_debug_mode =
                [](TFGrapplerDeviceGraph* graph) noexcept
            {
                TFGrapplerDeviceGraphOps::from_handle(graph).enable_debug_mode();
            },
            .debug_dump =
                [](TFGrapplerDeviceGraph* graph,
                   const TF_String* path,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerDeviceGraphOps::from_handle(graph).debug_dump(
                    ice::sonic::String::wrap(path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGrapplerDeviceGraphOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGrapplerDeviceGraph& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGrapplerDeviceGraphOps m_vtable;
    TFGrapplerDeviceGraph m_handle;
};

} // namespace ice::builder
