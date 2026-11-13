module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/function.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/op_def.pb.h"

export module aten_xpu_extern_grappler:function_inlining_pass;

import std;
import :graph_index;
import :graph_editor;
import :pass_context;

export namespace aten_xpu {

// Inlines calls to small functions from the graph's function library: a direct call (an op named
// after the function) or a PartitionedCall. The body nodes are copied under the call's name, the
// function inputs become the call's inputs and the readers of the call's outputs read the body
// values the function returns. A call is skipped when it has control edges or is preserved, the
// function has control returns, is large, calls itself, or an output port cannot be resolved.
class SyclFunctionInliningPass
{
public:
    explicit SyclFunctionInliningPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclFunctionInliningPass(const SyclFunctionInliningPass&) = delete;
    SyclFunctionInliningPass& operator=(const SyclFunctionInliningPass&) = delete;
    SyclFunctionInliningPass(SyclFunctionInliningPass&&) = delete;
    SyclFunctionInliningPass& operator=(SyclFunctionInliningPass&&) = delete;

    // Repeats until no call is left to inline, so nested calls unfold too.
    bool run(tensorflow::GraphDef& graph)
    {
        bool changed = false;
        for (int round = 0; round < k_max_rounds; ++round) {
            if (!inlineRound(graph)) {
                break;
            }
            changed = true;
        }
        return changed;
    }

private:
    static constexpr int k_max_rounds = 8;
    static constexpr int k_max_body_nodes = 128;

    bool inlineRound(tensorflow::GraphDef& graph)
    {
        if (graph.library().function_size() == 0) {
            return false;
        }
        m_scratch_nodes.clear();
        m_signatures.clear();
        for (const auto& function: graph.library().function()) {
            m_signatures.emplace(function.signature().name(), function.signature());
        }
        SyclGraphEditor editor{graph};
        {
            const SyclGraphIndex index{graph};
            for (int position = 0; position < index.size(); ++position) {
                const tensorflow::NodeDef& call = index.node(position);
                const auto* function = functionOf(graph, call);
                if (function != nullptr && inlineable(index, position, *function)) {
                    inlineCall(index, position, *function, editor);
                }
            }
        }
        const bool changed = editor.apply();
        for (const auto& node: m_scratch_nodes) {
            *graph.add_node() = node;
        }
        return changed;
    }

    const tensorflow::FunctionDef*
    functionOf(const tensorflow::GraphDef& graph, const tensorflow::NodeDef& call) const
    {
        std::string name = call.op();
        if (name == "PartitionedCall" || name == "StatefulPartitionedCall") {
            const auto target = call.attr().find("f");
            if (target == call.attr().end()) {
                return nullptr;
            }
            name = target->second.func().name();
        }
        for (const auto& function: graph.library().function()) {
            if (function.signature().name() == name) {
                return &function;
            }
        }
        return nullptr;
    }

    bool inlineable(
        const SyclGraphIndex& index,
        int position,
        const tensorflow::FunctionDef& function
    ) const
    {
        const tensorflow::NodeDef& call = index.node(position);
        if (m_context.isPreserved(call.name()) || index.hasControlEdges(position) ||
            function.control_ret_size() > 0 || function.node_def_size() > k_max_body_nodes ||
            call.input_size() != function.signature().input_arg_size()) {
            return false;
        }
        const bool simple_arguments =
            std::ranges::none_of(
                function.signature().input_arg(),
                [](const tensorflow::OpDef::ArgDef& argument)
                {
                    return !argument.number_attr().empty() || !argument.type_list_attr().empty();
                }
            ) &&
            std::ranges::none_of(
                function.signature().output_arg(),
                [](const tensorflow::OpDef::ArgDef& argument)
                {
                    return !argument.number_attr().empty() || !argument.type_list_attr().empty();
                }
            );
        const bool recursive = std::ranges::any_of(
            function.node_def(),
            [&function](const tensorflow::NodeDef& body)
            {
                return body.op() == function.signature().name();
            }
        );
        return simple_arguments && !recursive && resolvable(function);
    }

    // Every returned value must name a body node output whose port is known.
    bool resolvable(const tensorflow::FunctionDef& function) const
    {
        return std::ranges::all_of(
            function.signature().output_arg(),
            [this, &function](const tensorflow::OpDef::ArgDef& output)
            {
                const auto returned = function.ret().find(output.name());
                return returned != function.ret().end() &&
                       translate(function, returned->second, "").has_value();
            }
        );
    }

    void inlineCall(
        const SyclGraphIndex& index,
        int position,
        const tensorflow::FunctionDef& function,
        SyclGraphEditor& editor
    )
    {
        const tensorflow::NodeDef& call = index.node(position);
        const std::string prefix = call.name() + "/";
        for (const auto& body: function.node_def()) {
            tensorflow::NodeDef copy = body;
            copy.set_name(prefix + body.name());
            if (copy.device().empty()) {
                copy.set_device(call.device());
            }
            copy.clear_input();
            for (const auto& input: body.input()) {
                copy.add_input(bodyInput(function, call, input, prefix));
            }
            m_scratch_nodes.push_back(std::move(copy));
        }
        for (int slot = 0; slot < function.signature().output_arg_size(); ++slot) {
            const auto& output = function.signature().output_arg(slot);
            const auto value = translate(function, function.ret().at(output.name()), prefix);
            editor.redirect(call.name(), slot, bodyInputOrArgument(function, call, *value));
        }
        editor.remove(call.name());
    }

    // A body input in graph form: function inputs become the call's inputs, node outputs get the
    // prefix and their port.
    std::string bodyInput(
        const tensorflow::FunctionDef& function,
        const tensorflow::NodeDef& call,
        const std::string& input,
        const std::string& prefix
    ) const
    {
        if (input.starts_with('^')) {
            return "^" + prefix + input.substr(1);
        }
        const auto translated = translate(function, input, prefix);
        return bodyInputOrArgument(function, call, translated.value_or(input));
    }

    // A translated name is either a graph name ("call/node:port") or still a function input name.
    std::string bodyInputOrArgument(
        const tensorflow::FunctionDef& function,
        const tensorflow::NodeDef& call,
        const std::string& value
    ) const
    {
        for (int slot = 0; slot < function.signature().input_arg_size(); ++slot) {
            if (function.signature().input_arg(slot).name() == value) {
                return call.input(slot);
            }
        }
        return value;
    }

    // "node:output_arg:index" in a function body becomes "<prefix>node:port"; a bare function input
    // name stays as it is. Nothing when the node's op has no known output layout.
    std::optional<std::string> translate(
        const tensorflow::FunctionDef& function,
        const std::string& reference,
        const std::string& prefix
    ) const
    {
        const auto first = reference.find(':');
        if (first == std::string::npos) {
            return reference;
        }
        const auto second = reference.find(':', first + 1);
        if (second == std::string::npos) {
            return std::nullopt;
        }
        const std::string node_name = reference.substr(0, first);
        const std::string output_name = reference.substr(first + 1, second - first - 1);
        int index = 0;
        const auto digits = std::string_view{reference}.substr(second + 1);
        const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), index);
        if (parsed.ec != std::errc{}) {
            return std::nullopt;
        }
        const auto port = portOf(function, node_name, output_name, index);
        return port ? std::optional<std::string>{prefix + node_name + ":" + std::to_string(*port)}
                    : std::nullopt;
    }

    std::optional<int> portOf(
        const tensorflow::FunctionDef& function,
        const std::string& node_name,
        const std::string& output_name,
        int index
    ) const
    {
        const auto body = std::ranges::find_if(
            function.node_def(),
            [&node_name](const tensorflow::NodeDef& node)
            {
                return node.name() == node_name;
            }
        );
        if (body == function.node_def().end()) {
            return std::nullopt;
        }
        // A nested call has its signature in the library; any other op has the host's definition.
        const auto nested = m_signatures.find(body->op());
        const auto host = nested == m_signatures.end()
                              ? m_context.getLookup().definition(body->op())
                              : std::optional<tensorflow::OpDef>{nested->second};
        if (!host) {
            return std::nullopt;
        }
        int base = 0;
        for (const auto& output: host->output_arg()) {
            if (output.name() == output_name) {
                return base + index;
            }
            base += argumentSize(*body, output);
        }
        return std::nullopt;
    }

    static int argumentSize(const tensorflow::NodeDef& node, const tensorflow::OpDef::ArgDef& arg)
    {
        if (!arg.number_attr().empty()) {
            const auto count = node.attr().find(arg.number_attr());
            return count == node.attr().end() ? 1 : static_cast<int>(count->second.i());
        }
        if (!arg.type_list_attr().empty()) {
            const auto types = node.attr().find(arg.type_list_attr());
            return types == node.attr().end() ? 1 : types->second.list().type_size();
        }
        return 1;
    }

    SyclPassContext& m_context;
    std::vector<tensorflow::NodeDef> m_scratch_nodes;
    std::map<std::string, tensorflow::OpDef> m_signatures;
};

} // namespace aten_xpu
