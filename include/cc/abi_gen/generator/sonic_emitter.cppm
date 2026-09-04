export module cc_abi_gen_generator:sonic_emitter;

import std;
import cc_abi_gen_parser;
import :type_registry;
import :slot_classifier;

export namespace cc_abi_gen {

// Renders the sonic-tier module (include/cc/abi/sonic/<domain>/<domain>.cppm): a concrete
// Runtime<T, OpsStruct> subclass forwarding each method through the C vtable pointer.
// Everything is appended straight into m_writer's one continuous stream — no fragment strings
// are ever assembled first and interpolated in afterward, comma-joined lists included.
class SonicEmitter
{
public:
    explicit SonicEmitter(TypeRegistry &type_registry) noexcept : m_classifier(type_registry)
    {
    }

    std::string render(const VtableModel &model)
    {
        m_writer.clear();

        m_writer += std::format(
            R"cpp(module;

#include "c/extern/{0}/{0}.h"

export module cc_abi_sonic_{0};

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {{

class {1} : public ice::sonic::Runtime<{1}, {2}>
{{
public:
explicit {1}({2}* ops, void* plugin_context) noexcept : Runtime(ops, plugin_context) {{}}

static constexpr std::string_view domain_name = "{0}";

)cpp",
            model.m_domain_name,
            model.m_class_name,
            model.m_struct_name
        );

        for (const VtableSlot &slot : model.m_slots) {

            if (m_classifier.is_destroy(slot) || m_classifier.is_get_name(slot)) {

                continue;
            }

            write_method(slot);
        }

        m_writer += R"cpp(
ice::String get_name() const noexcept
{
ice::String out;
m_ops->get_name(get_handle(), out.get_handle());
return out;
}
};

} // namespace ice::sonic
)cpp";

        return m_writer;
    }

private:
    void write_method(const VtableSlot &slot)
    {
        std::span<const Parameter> middle = m_classifier.middle_parameters(slot);

        m_writer += std::format("[[nodiscard]] std::expected<void, ice::Status> {}(", slot.m_name);
        write_cpp_parameter_list(middle);
        m_writer += std::format(
            R"cpp() noexcept
{{
ice::Status status;
m_ops->{}(get_handle(), )cpp",
            slot.m_name
        );
        write_call_arguments(middle);
        m_writer += R"cpp(status.get_handle());
if (!status.ok()) {
return std::unexpected{status};
}
return {};
}

)cpp";
    }

    void write_cpp_parameter_list(std::span<const Parameter> parameters)
    {
        for (std::size_t index = 0; index < parameters.size(); ++index) {

            if (index != 0) {

                m_writer += ", ";
            }
            m_writer += std::format(
                "{} {}", m_classifier.cpp_parameter_type(parameters[index]), parameters[index].m_name
            );
        }
    }

    // Each entry is written as "<arg>, " (trailing comma+space), so the call site can follow
    // straight on with a fixed final argument (status.get_handle()) with no special-casing for
    // zero middle parameters.
    void write_call_arguments(std::span<const Parameter> parameters)
    {
        for (const Parameter &parameter : parameters) {

            m_writer += m_classifier.unwrap_argument(parameter.m_name, parameter.m_type) + ", ";
        }
    }

    SlotClassifier m_classifier;
    std::string m_writer;
};

} // namespace cc_abi_gen
