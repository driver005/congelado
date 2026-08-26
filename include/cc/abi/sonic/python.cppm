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

    static void set_attr(
        TF_Graph* graph,
        TF_Operation* op,
        const StringRuntime& attr_name,
        TF_Buffer* attr_value_proto,
        TF_Status* status
    )
    {

        tensorflow::SetAttr(graph, op, attr_name.c_str(), attr_value_proto, status);
    }

    static void clear_attr(
        TF_Graph* graph, TF_Operation* op, const StringRuntime& attr_name, TF_Status* status
    )
    {

        tensorflow::ClearAttr(graph, op, attr_name.c_str(), status);
    }

    static void set_full_type(TF_Graph* graph, TF_Operation* op, const TF_Buffer* full_type_proto)
    {

        tensorflow::SetFullType(graph, op, full_type_proto);
    }

    static void
    set_requested_device(TF_Graph* graph, TF_Operation* op, const StringRuntime& device)
    {

        tensorflow::SetRequestedDevice(graph, op, device.c_str());
    }

    static void update_edge(TF_Graph* graph, TF_Output new_src, TF_Input dst, TF_Status* status)
    {

        tensorflow::UpdateEdge(graph, new_src, dst, status);
    }

    static void extend_session(TF_Session* session, TF_Status* status)
    {

        tensorflow::ExtendSession(session, status);
    }

    static std::string get_handle_shape_and_type(TF_Graph* graph, TF_Output output)
    {

        return tensorflow::GetHandleShapeAndType(graph, output);
    }

    static void set_handle_shape_and_type(
        TF_Graph* graph, TF_Output output, const void* proto, size_t proto_len, TF_Status* status
    )
    {

        tensorflow::SetHandleShapeAndType(graph, output, proto, proto_len, status);
    }

    static void
    add_while_input_hack(TF_Graph* graph, TF_Output new_src, TF_Operation* dst, TF_Status* status)
    {

        tensorflow::AddWhileInputHack(graph, new_src, dst, status);
    }
};

} // namespace ice::sonic
