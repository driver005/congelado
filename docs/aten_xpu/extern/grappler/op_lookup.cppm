module;

#include "cc/proto/op_def.pb.h"
#include "include/c/extern/grappler/function_library.h"
#include "include/c/intern/buffer.h"

export module aten_xpu_extern_grappler:op_lookup;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// Looks up op definitions in the host's registry through the grappler function library ops, so a
// rewrite can check that the op it emits exists and takes the attrs and inputs it sets. The
// library is bound to the graph being optimized.
class SyclOpLookup
{
public:
    SyclOpLookup(const SyclOpsTable& ops, const ice::sonic::Buffer& graph_buf) :
        m_library{ops.getGrapplerFunctionLibraryOps()},
        m_status{ops.getStatusOps()},
        m_name{ops.getStringOps()},
        m_buffer{ops.getBufferOps()}
    {
        m_status.create();
        m_name.create();
        m_buffer.create();
        if (m_library.get_ops() != nullptr) {
            m_library.create();
            m_library.bind(graph_buf, m_status);
            m_bound = succeeded();
        }
    }

    ~SyclOpLookup()
    {
        if (m_library.get_ops() != nullptr) {
            m_library.destroy();
        }
        m_buffer.destroy();
        m_name.destroy();
        m_status.destroy();
    }

    SyclOpLookup(const SyclOpLookup&) = delete;
    SyclOpLookup& operator=(const SyclOpLookup&) = delete;
    SyclOpLookup(SyclOpLookup&&) = delete;
    SyclOpLookup& operator=(SyclOpLookup&&) = delete;

    // The host serves op definitions and the library is bound to the graph.
    bool available() const noexcept
    {
        return m_library.get_ops() != nullptr && m_bound;
    }

    // The op's definition as the host registered it.
    std::optional<tensorflow::OpDef> definition(std::string_view op)
    {
        return available() ? find(op) : std::nullopt;
    }

    // The op's is_stateful flag, or nothing when the host does not define the op.
    std::optional<bool> isStateful(std::string_view op)
    {
        const auto& definition = find(op);
        if (!definition) {
            return std::nullopt;
        }
        return definition->is_stateful();
    }

    // True when the op is defined and declares every named attr and input arg.
    bool defines(
        std::string_view op,
        std::initializer_list<std::string_view> attributes,
        std::initializer_list<std::string_view> inputs
    )
    {
        const auto& definition = find(op);
        if (!definition) {
            return false;
        }
        const bool has_attributes = std::ranges::all_of(
            attributes,
            [&definition](std::string_view name)
            {
                return std::ranges::any_of(
                    definition->attr(),
                    [name](const tensorflow::OpDef::AttrDef& attribute)
                    {
                        return attribute.name() == name;
                    }
                );
            }
        );
        const bool has_inputs = std::ranges::all_of(
            inputs,
            [&definition](std::string_view name)
            {
                return std::ranges::any_of(
                    definition->input_arg(),
                    [name](const tensorflow::OpDef::ArgDef& argument)
                    {
                        return argument.name() == name;
                    }
                );
            }
        );
        return has_attributes && has_inputs;
    }

private:
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

    const std::optional<tensorflow::OpDef>& find(std::string_view op)
    {
        if (!available()) {
            return m_none;
        }
        const auto cached = m_cache.find(std::string{op});
        if (cached != m_cache.end()) {
            return cached->second;
        }

        std::optional<tensorflow::OpDef> definition;
        m_name.copy(op);
        m_library.look_up_op_def(m_name, m_buffer, m_status);
        ice::sonic::Code code{ice::sonic::Code::k_ok};
        m_status.get_code(code);
        if (code.get_value() != TF_OK) {
            m_status.destroy();
            m_status.create();
        } else {
            ::TFBufferData data{};
            m_buffer.get_buffer(&data);
            tensorflow::OpDef parsed;
            if (parsed.ParseFromArray(data.data, static_cast<int>(data.length))) {
                definition = std::move(parsed);
            }
        }
        return m_cache.emplace(std::string{op}, std::move(definition)).first->second;
    }

    ice::sonic::GrapplerFunctionLibrary m_library;
    ice::sonic::Status m_status;
    ice::sonic::String m_name;
    ice::sonic::Buffer m_buffer;
    bool m_bound{true};
    const std::optional<tensorflow::OpDef> m_none;
    std::map<std::string, std::optional<tensorflow::OpDef>> m_cache;
};

} // namespace aten_xpu
