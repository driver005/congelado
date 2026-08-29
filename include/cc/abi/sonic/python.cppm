module;

#include "c/extern/python.h"

export module cc_abi_sonic:python;

import std;
import cc_abi_sonic_intern;

export namespace ice::sonic {

// Python API wrapper — these are C++ functions in the tensorflow namespace, called directly
// (no TP_* registration struct, no plugin/mainframe boundary to cross). Duplicated from
// PythonApiBuilder for pattern uniformity — behaviorally identical, same caveat as env/'s
// static-method classes.
class PythonApiRuntime
{
public:
    PythonApiRuntime() = default;
    ~PythonApiRuntime() = default;

    static void add_control_input(TF_Graph* graph, TF_Operation* op, TF_Operation* input)
    {

        tensorflow::AddControlInput(graph, op, input);
    }

    [[nodiscard]] static std::expected<void, Status> set_attr(
        TF_Graph* graph,
        TF_Operation* op,
        const String& attr_name,
        TF_Buffer_Handle* attr_value_proto
    )
    {

        Status status;
        tensorflow::SetAttr(graph, op, attr_name.c_str(), attr_value_proto, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, Status>
    clear_attr(TF_Graph* graph, TF_Operation* op, const String& attr_name)
    {

        Status status;
        tensorflow::ClearAttr(graph, op, attr_name.c_str(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    static void set_full_type(TF_Graph* graph, TF_Operation* op, const TF_Buffer_Handle* full_type_proto)
    {

        tensorflow::SetFullType(graph, op, full_type_proto);
    }

    static void
    set_requested_device(TF_Graph* graph, TF_Operation* op, const String& device)
    {

        tensorflow::SetRequestedDevice(graph, op, device.c_str());
    }

    [[nodiscard]] static std::expected<void, Status>
    update_edge(TF_Graph* graph, TF_Output new_src, TF_Input dst)
    {

        Status status;
        tensorflow::UpdateEdge(graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, Status> extend_session(TF_Session* session)
    {

        Status status;
        tensorflow::ExtendSession(session, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    static std::string get_handle_shape_and_type(TF_Graph* graph, TF_Output output)
    {

        return tensorflow::GetHandleShapeAndType(graph, output);
    }

    [[nodiscard]] static std::expected<void, Status> set_handle_shape_and_type(
        TF_Graph* graph, TF_Output output, const void* proto, size_t proto_len
    )
    {

        Status status;
        tensorflow::SetHandleShapeAndType(graph, output, proto, proto_len, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, Status>
    add_while_input_hack(TF_Graph* graph, TF_Output new_src, TF_Operation* dst)
    {

        Status status;
        tensorflow::AddWhileInputHack(graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
