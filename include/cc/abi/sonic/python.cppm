module;

#include "c/extern/python/python.h"

export module cc_abi_sonic:python;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::sonic {

// Runtime — the mainframe-facing Python API handle. Same resolve-by-name duality as
// ice::sonic::Logger and ice::sonic::Generator.
class PythonApi : public Runtime<PythonApi, TF_Python>
{
public:
    static constexpr std::string_view domain_name = "python";

    explicit PythonApi(TF_Python* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }

    void add_control_input(TF_Graph* graph, TF_Operation* op, TF_Operation* input) noexcept
    {
        m_ops->add_control_input(get_handle(), graph, op, input);
    }

    [[nodiscard]] std::expected<void, ice::Status> set_attr(
        TF_Graph* graph,
        TF_Operation* op,
        const ice::String& attr_name,
        TF_Buffer_Data* attr_value_proto
    ) noexcept
    {
        ice::Status status;
        m_ops->set_attr(
            get_handle(),
            graph,
            op,
            attr_name.get_handle(),
            attr_value_proto,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    clear_attr(TF_Graph* graph, TF_Operation* op, const ice::String& attr_name) noexcept
    {
        ice::Status status;
        m_ops->clear_attr(get_handle(), graph, op, attr_name.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void set_full_type(TF_Graph* graph, TF_Operation* op, const TF_Buffer_Data* full_type_proto) noexcept
    {
        m_ops->set_full_type(get_handle(), graph, op, full_type_proto);
    }

    void set_requested_device(TF_Graph* graph, TF_Operation* op, const ice::String& device) noexcept
    {
        m_ops->set_requested_device(get_handle(), graph, op, device.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::Status>
    update_edge(TF_Graph* graph, TF_Output new_src, TF_Input dst) noexcept
    {
        ice::Status status;
        m_ops->update_edge(get_handle(), graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> extend_session(TF_Session* session) noexcept
    {
        ice::Status status;
        m_ops->extend_session(get_handle(), session, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<TF_Buffer_Data*, ice::Status>
    get_handle_shape_and_type(TF_Graph* graph, TF_Output output) noexcept
    {
        TF_Buffer_Data* buffer = m_ops->get_handle_shape_and_type(get_handle(), graph, output);
        if (buffer == nullptr) {
            return std::unexpected{ice::Status{"handle shape/type unavailable"}};
        }
        return buffer;
    }

    [[nodiscard]] std::expected<void, ice::Status> set_handle_shape_and_type(
        TF_Graph* graph,
        TF_Output output,
        const void* proto,
        size_t proto_len
    ) noexcept
    {
        ice::Status status;
        m_ops->set_handle_shape_and_type(
            get_handle(),
            graph,
            output,
            proto,
            proto_len,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_while_input_hack(TF_Graph* graph, TF_Output new_src, TF_Operation* dst) noexcept
    {
        ice::Status status;
        m_ops->add_while_input_hack(get_handle(), graph, new_src, dst, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
