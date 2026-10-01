// SYCL reference plugin — command_graph capture/instantiate/replay.
//
// Not built by Bazel (docs/ only). Replaces core/XPUGraph.{h,cpp}. Simplified relative to the
// real XPUGraphImpl: only default capture mode's queue-state-based filter, no keep_graph=false
// auto-instantiate/reset-on-capture-end, no captured-generator-state bookkeeping beyond
// registering/unregistering with SyclRandomGenerator (whose graph-safe tensor state is not
// wired up yet — see random_generator.cppm).

module;

#include "include/c/extern/grappler/device_graph.h"

export module sycl_backend:device_graph;

import std;
import cc_ice_extern_grappler_builder;
import :stream;
import :random_generator;

export namespace sycl_backend {

class SyclDeviceGraph : public ice::builder::TFGrapplerDeviceGraph
{
public:
    SyclDeviceGraph() = default;

    ~SyclDeviceGraph() override = default;
    SyclDeviceGraph(const SyclDeviceGraph&) = delete;
    SyclDeviceGraph& operator=(const SyclDeviceGraph&) = delete;
    SyclDeviceGraph(SyclDeviceGraph&&) = delete;
    SyclDeviceGraph& operator=(SyclDeviceGraph&&) = delete;

    [[nodiscard]] std::expected<void, ice::sonic::Status> capture_begin(
        ice::builder::Stream& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode
    ) noexcept override
    {
        if (m_has_graph_exec) {
            return std::unexpected{ice::sonic::Status::from_message(
                "SyclDeviceGraph: instance already owns a captured graph"
            )};
        }

        if (mode != TF_CAPTURE_MODE_GLOBAL) {
            // c10/XPUGraph.cpp warns and falls back to default for ThreadLocal/Relaxed too; the
            // underlying sycl::ext::oneapi::experimental::command_graph only has one capture
            // behavior regardless.
        }

        auto* native_stream = dynamic_cast<SyclStream*>(&capture_stream);
        if (native_stream == nullptr) {
            return std::unexpected{ice::sonic::Status::from_message(
                "SyclDeviceGraph: capture stream was not created by this plugin"
            )};
        }

        m_capture_stream = native_stream;
        m_capture_stream->set_capturing(true);
        m_pool_id = pool_id != nullptr ? *pool_id : TF_PoolId{.first = 1, .second = 0};

        try {
            m_graph = sycl::ext::oneapi::experimental::command_graph{
                m_capture_stream->get_native_queue()
            };
            m_graph->begin_recording(m_capture_stream->get_native_queue());
        } catch (const sycl::exception& error) {
            m_capture_stream->set_capturing(false);
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> capture_end() noexcept override
    {
        if (!m_graph.has_value() || m_capture_stream == nullptr) {
            return std::unexpected{ice::sonic::Status::from_message(
                "SyclDeviceGraph: capture_end without capture_begin"
            )};
        }

        try {
            m_graph->end_recording();
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        m_capture_stream->set_capturing(false);
        m_capture_ended = true;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> instantiate() noexcept override
    {
        if (!m_capture_ended) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclDeviceGraph: instantiate before capture_end")
            };
        }

        try {
            m_graph_exec = m_graph->finalize();
            m_has_graph_exec = true;
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> replay() noexcept override
    {
        if (!m_capture_ended) {
            return std::unexpected{ice::sonic::Status::from_message(
                "SyclDeviceGraph: replay without a preceding capture"
            )};
        }

        if (!m_has_graph_exec) {
            auto instantiated = instantiate();
            if (!instantiated) {
                return std::unexpected{instantiated.error()};
            }
        }

        try {
            m_capture_stream->get_native_queue().ext_oneapi_graph(*m_graph_exec);
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> reset() noexcept override
    {
        m_graph_exec.reset();
        m_has_graph_exec = false;
        m_graph.reset();
        m_capture_ended = false;
        return {};
    }

    void get_pool(TF_PoolId* out_pool_id) noexcept override
    {
        *out_pool_id = m_pool_id;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    register_random_generator(ice::builder::RandomGenerator& generator) noexcept override
    {
        static_cast<SyclRandomGenerator&>(generator).register_graph(this);
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    unregister_random_generator(ice::builder::RandomGenerator& generator) noexcept override
    {
        static_cast<SyclRandomGenerator&>(generator).unregister_graph(this);
        return {};
    }

    void enable_debug_mode() noexcept override
    {
        m_debug_mode = true;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    debug_dump(const char* path) noexcept override
    {
        (void)path;

        if (!m_debug_mode || !m_graph.has_value()) {
            return {};
        }

        // sycl::ext::oneapi::experimental::command_graph has no portable dot/text dump API in
        // this SYCL version; a real backend would print_graph() to `path` here.
        return {};
    }

private:
    SyclStream* m_capture_stream{nullptr};
    TF_PoolId m_pool_id{};
    std::optional<sycl::ext::oneapi::experimental::command_graph<
        sycl::ext::oneapi::experimental::graph_state::modifiable>>
        m_graph;
    std::optional<sycl::ext::oneapi::experimental::command_graph<
        sycl::ext::oneapi::experimental::graph_state::executable>>
        m_graph_exec;
    bool m_capture_ended{false};
    bool m_has_graph_exec{false};
    bool m_debug_mode{false};
};

} // namespace sycl_backend
