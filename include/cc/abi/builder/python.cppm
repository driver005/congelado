module;

#include "c/abi/api.h"

export module cc_abi_builder:python;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Python API wrapper — thin adapters over the C ABI's TF_* graph/session functions
// (api.h), not a C ABI of their own. The previous version called tensorflow::* C++
// helpers that do not exist in this repo (include/c is declaration-only); these now
// call the declared TF_* entry points directly. All members are noexcept — the
// std::expected return is the only failure channel.
class PythonApiBuilder
{
public:
    PythonApiBuilder() noexcept = default;
    ~PythonApiBuilder() noexcept = default;

    // Graph mutation helpers
    static void add_control_input(TF_Graph* graph, TF_Operation* op, TF_Operation* input) noexcept
    {
        TF_AddOperationControlInput(graph, op, input);
    }

    [[nodiscard]] static std::expected<void, ice::Status> set_attr(
        TF_Graph* graph,
        TF_Operation* op,
        const ice::String& attr_name,
        TF_Buffer* attr_value_proto
    ) noexcept
    {
        ice::Status status;
        TF_SetAttr(graph, op, attr_name.c_str(), attr_value_proto, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, ice::Status>
    clear_attr(TF_Graph* graph, TF_Operation* op, const ice::String& attr_name) noexcept
    {
        ice::Status status;
        TF_ClearAttr(graph, op, attr_name.c_str(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    static void
    set_full_type(TF_Graph* graph, TF_Operation* op, const TF_Buffer* full_type_proto) noexcept
    {
        TF_SetFullType(graph, op, full_type_proto);
    }

    static void
    set_requested_device(TF_Graph* graph, TF_Operation* op, const ice::String& device) noexcept
    {
        TF_SetRequestedDevice(graph, op, device.c_str());
    }

    [[nodiscard]] static std::expected<void, ice::Status>
    update_edge(TF_Graph* graph, TF_Output new_src, TF_Input dst) noexcept
    {
        ice::Status status;
        TF_UpdateEdge(graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, ice::Status> extend_session(TF_Session* session) noexcept
    {
        ice::Status status;
        TF_ExtendSession(session, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    // The C ABI's TF_GetHandleShapeAndType returns a TF_Buffer whose backing runtime is not
    // linked into this repo (include/c is declaration-only), so the wrapper reports the
    // capability as unavailable rather than dereferencing an uninitialized buffer.
    [[nodiscard]] static std::expected<ice::String, ice::Status>
    get_handle_shape_and_type(TF_Graph* /*graph*/, TF_Output /*output*/) noexcept
    {
        return std::unexpected{ice::Status{"TF buffer runtime not linked in this build"}};
    }

    [[nodiscard]] static std::expected<void, ice::Status> set_handle_shape_and_type(
        TF_Graph* graph,
        TF_Output output,
        const void* proto,
        size_t proto_len
    ) noexcept
    {
        ice::Status status;
        TF_SetHandleShapeAndType(graph, output, proto, proto_len, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, ice::Status>
    add_while_input_hack(TF_Graph* graph, TF_Output new_src, TF_Operation* dst) noexcept
    {
        ice::Status status;
        TF_AddWhileInputHack(graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::builder
