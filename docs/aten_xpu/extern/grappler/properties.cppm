module;

#include "cc/proto/tensor_properties.pb.h"
#include "include/c/extern/grappler/properties.h"
#include "include/c/intern/buffer.h"

export module aten_xpu_extern_grappler:properties;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// Static shape inference over the graph being optimized, served by the host through the grappler
// properties ops, bound to the item being optimized.
class SyclGraphProperties
{
public:
    SyclGraphProperties(const SyclOpsTable& ops, const ice::sonic::GrapplerItem& item) :
        m_ops{ops},
        m_properties{ops.getGrapplerPropertiesOps()},
        m_status{ops.getStatusOps()},
        m_name{ops.getStringOps()}
    {
        m_status.create();
        m_name.create();
        if (available()) {
            m_properties.create();
            m_bound = false;
            m_properties.bind(item, m_status);
            m_bound = succeeded();
        }
    }

    ~SyclGraphProperties()
    {
        if (m_properties.get_ops() != nullptr) {
            m_properties.destroy();
        }
        m_name.destroy();
        m_status.destroy();
    }

    SyclGraphProperties(const SyclGraphProperties&) = delete;
    SyclGraphProperties& operator=(const SyclGraphProperties&) = delete;
    SyclGraphProperties(SyclGraphProperties&&) = delete;
    SyclGraphProperties& operator=(SyclGraphProperties&&) = delete;

    // The host serves properties and the handle is bound to the item.
    bool available() const noexcept
    {
        return m_properties.get_ops() != nullptr && m_bound;
    }

    // Runs the host's shape inference, which calls the shape function of every op in the graph.
    bool infer()
    {
        if (!available()) {
            return false;
        }
        m_properties.infer_statically(true, false, false, false, m_status);
        m_inferred = succeeded();
        return m_inferred;
    }

    bool isInferred() const noexcept
    {
        return m_inferred;
    }

    // How many outputs the host's inference reports for the node; 0 when unknown.
    int outputCount(std::string_view node)
    {
        if (!m_inferred) {
            return 0;
        }
        m_name.copy(node);
        int count = 0;
        m_properties.get_output_properties_size(m_name, count, m_status);
        return succeeded() ? count : 0;
    }

    std::optional<tensorflow::TensorProperties> input(std::string_view node, int port)
    {
        return fetch(node, port, true);
    }

    std::optional<tensorflow::TensorProperties> output(std::string_view node, int port)
    {
        return fetch(node, port, false);
    }

private:
    std::optional<tensorflow::TensorProperties> fetch(std::string_view node, int port, bool inputs)
    {
        if (!m_inferred || port < 0) {
            return std::nullopt;
        }
        m_name.copy(node);
        int count = 0;
        if (inputs) {
            m_properties.get_input_properties_size(m_name, count, m_status);
        } else {
            m_properties.get_output_properties_size(m_name, count, m_status);
        }
        if (!succeeded() || port >= count) {
            return std::nullopt;
        }

        m_scratch_buffers.clear();
        m_scratch_handles.assign(static_cast<std::size_t>(count), nullptr);
        for (int index = 0; index < count; ++index) {
            auto& buffer = m_scratch_buffers.emplace_back(m_ops.getBufferOps());
            buffer.create();
            m_scratch_handles[static_cast<std::size_t>(index)] = buffer.get_handle();
        }
        if (inputs) {
            m_properties.get_input_properties(m_name, m_scratch_handles.data(), count, m_status);
        } else {
            m_properties.get_output_properties(m_name, m_scratch_handles.data(), count, m_status);
        }

        std::optional<tensorflow::TensorProperties> result;
        if (succeeded()) {
            ::TFBufferData data{};
            m_scratch_buffers[static_cast<std::size_t>(port)].get_buffer(&data);
            tensorflow::TensorProperties parsed;
            if (parsed.ParseFromArray(data.data, static_cast<int>(data.length))) {
                result = std::move(parsed);
            }
        }
        for (const auto& buffer: m_scratch_buffers) {
            buffer.destroy();
        }
        m_scratch_buffers.clear();
        return result;
    }

    // Reads the status and clears it after a failure so the next call starts clean.
    bool succeeded()
    {
        ice::sonic::Code code{ice::sonic::Code::k_ok};
        m_status.get_code(code);
        if (code.get_value() == TF_OK) {
            return true;
        }
        m_status.destroy();
        m_status.create();
        return false;
    }

    const SyclOpsTable& m_ops;
    ice::sonic::GrapplerProperties m_properties;
    ice::sonic::Status m_status;
    ice::sonic::String m_name;
    bool m_bound{true};
    bool m_inferred{false};
    std::deque<ice::sonic::Buffer> m_scratch_buffers;
    std::vector<::TF_Buffer*> m_scratch_handles;
};

} // namespace aten_xpu
