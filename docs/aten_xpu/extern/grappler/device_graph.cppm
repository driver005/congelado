module;

#include "include/c/extern/grappler/device_graph.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_grappler:device_graph;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_extern_grappler_sonic;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_builder;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;
import aten_xpu_extern_random_generator;
import :capture_status;

export namespace aten_xpu {

class SyclDeviceGraph : public ice::builder::GrapplerDeviceGraph
{
public:
    using ModifiableGraph = sycl::ext::oneapi::experimental::command_graph<
        sycl::ext::oneapi::experimental::graph_state::modifiable>;
    using ExecutableGraph = sycl::ext::oneapi::experimental::command_graph<
        sycl::ext::oneapi::experimental::graph_state::executable>;

    explicit SyclDeviceGraph(const SyclOpsTable& ops) noexcept :
        ice::builder::GrapplerDeviceGraph{
            ops.getRandomGeneratorOps(),
            ops.getStatusOps(),
            ops.getStreamOps(),
            ops.getStringOps()
        },
        m_status{ops},
        m_graph_state{ops.getGraphState()}
    {
    }

    ~SyclDeviceGraph() override = default;
    SyclDeviceGraph(const SyclDeviceGraph&) = delete;
    SyclDeviceGraph& operator=(const SyclDeviceGraph&) = delete;
    SyclDeviceGraph(SyclDeviceGraph&&) = delete;
    SyclDeviceGraph& operator=(SyclDeviceGraph&&) = delete;

    void bind(
        std::shared_ptr<SyclPhiloxState> default_generator,
        const sycl::device& device,
        int device_index
    )
    {
        m_default_generator = std::move(default_generator);
        m_device = device;
        m_device_index = device_index;
        m_pool_id = ice::sonic::PoolId{static_cast<int64_t>(m_graph_state.next_pool_sequence()), 0};
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void capture_begin(
        const ice::sonic::Stream& capture_stream,
        const ice::sonic::PoolId* pool_id,
        ice::sonic::CaptureMode mode,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(mode);
        if (m_graph_exec) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                "graph instance already owns a capture"
            );
            return;
        }

        auto& stream =
            static_cast<SyclStream&>(SyclStream::from_handle(capture_stream.get_handle()));
        m_capture_stream = stream.getSharedQueue();
        if (pool_id != nullptr && (pool_id->getFirst() != 0 || pool_id->getSecond() != 0)) {
            m_pool_id = *pool_id;
        }

        try {
            if (m_default_generator) {
                m_generator_states.try_emplace(m_default_generator, 0);
                m_default_generator->register_graph(this);
            }
            for (auto& [state, increment]: m_generator_states) {
                state->capture_prologue(*m_capture_stream);
            }

            m_graph.emplace(*m_capture_stream, recording_properties());
            m_graph->begin_recording(*m_capture_stream);
            stream.setCapturing(true);
            m_capture_owner = stream;
        } catch (const std::exception& error) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, error.what());
        }
    }

    void capture_end(const ice::sonic::Status& out_status) noexcept override
    {
        if (!m_graph || !m_capture_stream) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                "capture_end without capture_begin"
            );
            return;
        }

        try {
            m_graph->end_recording();
            if (m_capture_owner) {
                m_capture_owner->get().setCapturing(false);
                m_capture_owner.reset();
            }
            for (auto& [state, increment]: m_generator_states) {
                increment = state->capture_epilogue();
            }
            m_capture_ended = true;
            if (!m_graph_state.getDebugMode()) {
                instantiate(out_status);
            }
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void instantiate(const ice::sonic::Status& out_status) noexcept override
    {
        if (!m_capture_ended || !m_graph) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                "instantiate before capture_end"
            );
            return;
        }

        try {
            m_graph_exec.emplace(m_graph->finalize());
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void replay(const ice::sonic::Status& out_status) noexcept override
    {
        if (!m_capture_ended) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                "replay without a capture"
            );
            return;
        }
        if (!m_graph_exec) {
            instantiate(out_status);
            if (!m_graph_exec) {
                return;
            }
        }

        try {
            for (auto& [state, increment]: m_generator_states) {
                state->replay_prologue(*m_capture_stream, increment);
            }
            m_capture_stream->ext_oneapi_graph(*m_graph_exec);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void reset(const ice::sonic::Status& out_status) noexcept override
    {
        static_cast<void>(out_status);
        for (auto& [state, increment]: m_generator_states) {
            try {
                state->unregister_graph(this);
            } catch (const std::logic_error&) {
            }
        }
        m_generator_states.clear();
        m_graph_exec.reset();
        m_graph.reset();
        m_capture_ended = false;
    }

    void get_pool(ice::sonic::PoolId* out_pool_id) noexcept override
    {
        *out_pool_id = m_pool_id;
    }

    void register_random_generator(
        const ice::sonic::RandomGenerator& generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            const auto& state = static_cast<SyclRandomGenerator&>(
                                    SyclRandomGenerator::from_handle(generator.get_handle())
            )
                                    .getSharedState();
            state->register_graph(this);
            m_generator_states.try_emplace(state, 0);
        } catch (const std::logic_error& error) {
            m_status.fail(out_status, ice::sonic::Code::k_failed_precondition, error.what());
        }
    }

    void unregister_random_generator(
        const ice::sonic::RandomGenerator& generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            const auto& state = static_cast<SyclRandomGenerator&>(
                                    SyclRandomGenerator::from_handle(generator.get_handle())
            )
                                    .getSharedState();
            state->unregister_graph(this);
            m_generator_states.erase(state);
        } catch (const std::logic_error& error) {
            m_status.fail(out_status, ice::sonic::Code::k_failed_precondition, error.what());
        }
    }

    void enable_debug_mode() noexcept override
    {
        m_graph_state.setDebugMode(true);
    }

    void debug_dump(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        const char* data = nullptr;
        std::size_t size = 0;
        path.get_data_pointer(&data);
        path.get_size(size);
        const std::string target(data, size);
        if (!target.ends_with(".dot")) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "debug path must end with .dot"
            );
            return;
        }
        if (!m_graph) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                "no captured graph to dump"
            );
            return;
        }

        try {
            m_graph->print_graph(target, true);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    sycl::queue* getCaptureQueue() const noexcept
    {
        return m_capture_stream.get();
    }

private:
    sycl::property_list recording_properties() const
    {
        namespace experimental = sycl::ext::oneapi::experimental;

        const auto architecture = m_device.get_info<experimental::info::device::architecture>();
        if (architecture == experimental::architecture::intel_gpu_pvc ||
            architecture == experimental::architecture::intel_gpu_pvc_vg) {
            return {};
        }
        return {experimental::property::graph::enable_native_recording{}};
    }

    SyclStatus m_status;
    SyclGraphState& m_graph_state;
    sycl::device m_device;
    int m_device_index{0};
    ice::sonic::PoolId m_pool_id{};
    std::shared_ptr<sycl::queue> m_capture_stream;
    std::optional<std::reference_wrapper<SyclStream>> m_capture_owner;
    std::shared_ptr<SyclPhiloxState> m_default_generator;
    std::map<std::shared_ptr<SyclPhiloxState>, uint64_t> m_generator_states;
    std::optional<ModifiableGraph> m_graph;
    std::optional<ExecutableGraph> m_graph_exec;
    bool m_capture_ended{false};
};

} // namespace aten_xpu
