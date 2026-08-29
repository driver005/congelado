module;

#include "c/abi/api.h"

export module cc_abi_builder:python;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Python API wrapper - these are C++ functions in the tensorflow namespace
// primarily used for SWIG/Python bindings. Not a C ABI.

class PythonApiBuilder
{
public:
    PythonApiBuilder() = default;
    ~PythonApiBuilder() = default;

    // Graph mutation helpers
    static void add_control_input(TF_Graph* graph, TF_Operation* op, TF_Operation* input)
    {
        tensorflow::AddControlInput(graph, op, input);
    }

    [[nodiscard]] static std::expected<void, ice::Status> set_attr(
        TF_Graph* graph,
        TF_Operation* op,
        const ice::String& attr_name,
        TF_Buffer_Handle* attr_value_proto
    )
    {
        ice::Status status;
        tensorflow::SetAttr(graph, op, attr_name.c_str(), attr_value_proto, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, ice::Status>
    clear_attr(TF_Graph* graph, TF_Operation* op, const ice::String& attr_name)
    {
        ice::Status status;
        tensorflow::ClearAttr(graph, op, attr_name.to_std_string().data(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    static void
    set_full_type(TF_Graph* graph, TF_Operation* op, const TF_Buffer_Handle* full_type_proto)
    {
        tensorflow::SetFullType(graph, op, full_type_proto);
    }

    static void set_requested_device(TF_Graph* graph, TF_Operation* op, const ice::String& device)
    {
        tensorflow::SetRequestedDevice(graph, op, device.c_str());
    }

    [[nodiscard]] static std::expected<void, ice::Status>
    update_edge(TF_Graph* graph, TF_Output new_src, TF_Input dst)
    {
        ice::Status status;
        tensorflow::UpdateEdge(graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, ice::Status> extend_session(TF_Session* session)
    {
        ice::Status status;
        tensorflow::ExtendSession(session, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    static ice::String get_handle_shape_and_type(TF_Graph* graph, TF_Output output)
    {
        return ice::String{tensorflow::GetHandleShapeAndType(graph, output)};
    }

    [[nodiscard]] static std::expected<void, ice::Status> set_handle_shape_and_type(
        TF_Graph* graph,
        TF_Output output,
        const void* proto,
        size_t proto_len
    )
    {
        ice::Status status;
        tensorflow::SetHandleShapeAndType(graph, output, proto, proto_len, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, ice::Status>
    add_while_input_hack(TF_Graph* graph, TF_Output new_src, TF_Operation* dst)
    {
        ice::Status status;
        tensorflow::AddWhileInputHack(graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::builder
