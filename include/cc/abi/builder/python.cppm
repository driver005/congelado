module;

#include "c/extern/python/python.h"

export module cc_abi_builder:python;

import std;
import cc_abi_primitives;

export namespace ice::builder {

// Abstract base class for a Python API backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Logger's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::Registration under type="python".
class PythonApi
{
public:
    // Recover the PythonApi instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static PythonApi* create(void* ctx) noexcept
    {
        return static_cast<PythonApi*>(ctx);
    }

    virtual ~PythonApi() = default;

    virtual ice::String get_name() const noexcept = 0;

    virtual void
    add_control_input(TF_Graph* graph, TF_Operation* op, TF_Operation* input) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> set_attr(
        TF_Graph* graph,
        TF_Operation* op,
        const ice::String& attr_name,
        TF_Buffer_Data* attr_value_proto
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    clear_attr(TF_Graph* graph, TF_Operation* op, const ice::String& attr_name) noexcept = 0;

    virtual void set_full_type(
        TF_Graph* graph,
        TF_Operation* op,
        const TF_Buffer_Data* full_type_proto
    ) noexcept = 0;

    virtual void
    set_requested_device(TF_Graph* graph, TF_Operation* op, const ice::String& device) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    update_edge(TF_Graph* graph, TF_Output new_src, TF_Input dst) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    extend_session(TF_Session* session) noexcept = 0;

    [[nodiscard]] virtual std::expected<TF_Buffer_Data*, ice::Status>
    get_handle_shape_and_type(TF_Graph* graph, TF_Output output) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> set_handle_shape_and_type(
        TF_Graph* graph,
        TF_Output output,
        const void* proto,
        size_t proto_len
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_while_input_hack(TF_Graph* graph, TF_Output new_src, TF_Operation* dst) noexcept = 0;

    static TF_Python* get_generic_vtable()
    {
        static TF_Python vtable = {
            .struct_size = TF_PYTHON_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete PythonApi::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                PythonApi::create(plugin_context)->get_name().to_c(out);
            },
            .add_control_input =
                [](void* plugin_context, TF_Graph* graph, TF_Operation* op, TF_Operation* input) noexcept
            {
                PythonApi::create(plugin_context)->add_control_input(graph, op, input);
            },
            .set_attr =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Operation* op,
                   const TF_TString* attr_name,
                   TF_Buffer_Data* attr_value_proto,
                   TF_Status* status) noexcept
            {
                auto res = PythonApi::create(plugin_context)
                               ->set_attr(graph, op, ice::String::create(attr_name), attr_value_proto);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear_attr =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Operation* op,
                   const TF_TString* attr_name,
                   TF_Status* status) noexcept
            {
                auto res = PythonApi::create(plugin_context)
                               ->clear_attr(graph, op, ice::String::create(attr_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_full_type =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Operation* op,
                   const TF_Buffer_Data* full_type_proto) noexcept
            {
                PythonApi::create(plugin_context)->set_full_type(graph, op, full_type_proto);
            },
            .set_requested_device =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Operation* op,
                   const TF_TString* device) noexcept
            {
                PythonApi::create(plugin_context)
                    ->set_requested_device(graph, op, ice::String::create(device));
            },
            .update_edge =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Output new_src,
                   TF_Input dst,
                   TF_Status* status) noexcept
            {
                auto res = PythonApi::create(plugin_context)->update_edge(graph, new_src, dst);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .extend_session =
                [](void* plugin_context, TF_Session* session, TF_Status* status) noexcept
            {
                auto res = PythonApi::create(plugin_context)->extend_session(session);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_handle_shape_and_type =
                [](void* plugin_context, TF_Graph* graph, TF_Output output) noexcept
                -> TF_Buffer_Data*
            {
                auto res = PythonApi::create(plugin_context)->get_handle_shape_and_type(graph, output);
                if (!res) {
                    return nullptr; // no status slot — allocator contract
                }
                return res.value();
            },
            .set_handle_shape_and_type =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Output output,
                   const void* proto,
                   size_t proto_len,
                   TF_Status* status) noexcept
            {
                auto res = PythonApi::create(plugin_context)
                               ->set_handle_shape_and_type(graph, output, proto, proto_len);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_while_input_hack =
                [](void* plugin_context,
                   TF_Graph* graph,
                   TF_Output new_src,
                   TF_Operation* dst,
                   TF_Status* status) noexcept
            {
                auto res = PythonApi::create(plugin_context)->add_while_input_hack(graph, new_src, dst);
                if (!res) {
                    res.error().to_c(status);
                }
            },
        };
        return &vtable;
    }
};

} // namespace ice::builder
