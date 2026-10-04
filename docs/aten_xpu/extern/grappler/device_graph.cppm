module;

#include "include/c/extern/grappler/device_graph.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_grappler:device_graph;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_builder;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;
import aten_xpu_extern_random_generator;
import :capture_status;

export namespace aten_xpu {

class SyclDeviceGraph : public ice::builder::TFGrapplerDeviceGraphOps
{
public:
    using ModifiableGraph = sycl::ext::oneapi::experimental::command_graph<
        sycl::ext::oneapi::experimental::graph_state::modifiable>;
    using ExecutableGraph = sycl::ext::oneapi::experimental::command_graph<
        sycl::ext::oneapi::experimental::graph_state::executable>;

    explicit SyclDeviceGraph(const SyclOpsTable& ops) noexcept :
        ice::builder::TFGrapplerDeviceGraphOps{
            ops.getRandomGeneratorOps(),
            ops.getStatusOps(),
            ops.getStreamOps(),
            ops.getStringOps()
        },
        m_status{ops}
    {
    }

    ~SyclDeviceGraph() override = default;
    SyclDeviceGraph(const SyclDeviceGraph&) = delete;
    SyclDeviceGraph& operator=(const SyclDeviceGraph&) = delete;
    SyclDeviceGraph(SyclDeviceGraph&&) = delete;
    SyclDeviceGraph& operator=(SyclDeviceGraph&&) = delete;

    static void create(::TFGrapplerDeviceGraph* handle)
    {

        auto* graph = new SyclDeviceGraph{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *graph);

    }

    void bind(std::shared_ptr<SyclPhiloxState> default_generator, const sycl::device& device, int device_index)
    {

        m_default_generator = std::move(default_generator);
        m_device = device;
        m_device_index = device_index;
        m_pool_id = TF_PoolId{.first = static_cast<int64_t>(++s_graph_pool_sequence), .second = 0};

    }

    void destroy() noexcept override { delete this; }

    void capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(mode);
        if (m_graph_exec) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "graph instance already owns a capture");
            return;
        }

        auto& stream = SyclHandle::resolve<SyclStream>(capture_stream);
        m_capture_stream = stream.getSharedQueue();
        if (pool_id != nullptr && (pool_id->first != 0 || pool_id->second != 0)) {
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
            m_status.fail(out_status, TF_INTERNAL, error.what());
        }

    }

    void capture_end(const ice::sonic::Status& out_status) noexcept override
    {

        if (!m_graph || !m_capture_stream) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "capture_end without capture_begin");
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
            if (!s_debug_mode) {
                instantiate(out_status);
            }
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void instantiate(const ice::sonic::Status& out_status) noexcept override
    {

        if (!m_capture_ended || !m_graph) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "instantiate before capture_end");
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
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "replay without a capture");
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

    void get_pool(TF_PoolId* out_pool_id) noexcept override { *out_pool_id = m_pool_id; }

    void register_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            const auto& state = SyclHandle::resolve<SyclRandomGenerator>(generator).getSharedState();
            state->register_graph(this);
            m_generator_states.try_emplace(state, 0);
        } catch (const std::logic_error& error) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, error.what());
        }

    }

    void unregister_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            const auto& state = SyclHandle::resolve<SyclRandomGenerator>(generator).getSharedState();
            state->unregister_graph(this);
            m_generator_states.erase(state);
        } catch (const std::logic_error& error) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, error.what());
        }

    }

    void enable_debug_mode() noexcept override { s_debug_mode = true; }

    void debug_dump(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept override
    {

        const char* data = nullptr;
        std::size_t size = 0;
        path.get_data_pointer(&data);
        path.get_size(&size);
        const std::string target(data, size);
        if (!target.ends_with(".dot")) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "debug path must end with .dot");
            return;
        }
        if (!m_graph) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "no captured graph to dump");
            return;
        }

        try {
            m_graph->print_graph(target, true);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    sycl::queue* getCaptureQueue() const noexcept { return m_capture_stream.get(); }

private:
    sycl::property_list recording_properties() const
    {

        namespace experimental = sycl::ext::oneapi::experimental;

        const auto architecture = m_device.get_info<experimental::info::device::architecture>();
        if (architecture == experimental::architecture::intel_gpu_pvc ||
            architecture == experimental::architecture::intel_gpu_pvc_vg)
        {
            return {};
        }
        return {experimental::property::graph::enable_native_recording{}};

    }

    SyclStatus m_status;
    sycl::device m_device;
    int m_device_index{0};
    TF_PoolId m_pool_id{};
    std::shared_ptr<sycl::queue> m_capture_stream;
    std::optional<std::reference_wrapper<SyclStream>> m_capture_owner;
    std::shared_ptr<SyclPhiloxState> m_default_generator;
    std::map<std::shared_ptr<SyclPhiloxState>, uint64_t> m_generator_states;
    std::optional<ModifiableGraph> m_graph;
    std::optional<ExecutableGraph> m_graph_exec;
    bool m_capture_ended{false};

    static inline bool s_debug_mode{false};
    static inline std::uint64_t s_graph_pool_sequence{0};
};

} // namespace aten_xpu
