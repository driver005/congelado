export module cc_abi_gen_generator:sonic_emitter;

import std;
import cc_abi_gen_parser;
import :type_registry;
import :slot_classifier;
import :helper_formater;

export namespace cc_abi_gen::emitter {

class Sonic
{
public:
    Sonic(std::reference_wrapper<Register> registry, std::string_view namespace_name) :
        m_registry{registry},
        m_namespace_name{namespace_name}
    {
    }

    std::string render(const vtable::Model& model)
    {
        m_writer.clear();

        m_writer += helper::format_header(
            helper::GenTarget::Sonic,
            model.get_domain_name(),
            model.get_class_name(),
            m_namespace_name,
            model.get_struct_name()
        );

        for (const vtable::Slot& slot: model.get_slots()) {
            if (slot.is_destroy() || slot.is_get_name()) {
                continue;
            }

            write_method(slot);
        }

        m_writer += helper::format_sonic_footer();

        return m_writer;
    }

private:
    void write_method(const VtableSlot& slot)
    {
        std::span<const Parameter> middle = m_classifier.middle_parameters(slot);

        m_writer += helper::format_method_signature(slot.get_name(), m_namespace_name);

        write_cpp_parameter_list(middle);

        m_writer += helper::format_method_body_start(slot.get_name());

        write_call_arguments(middle);

        m_writer += helper::format_method_body_end();
    }

    std::expected<void, std::string> write_cpp_parameter_list(std::span<const Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto type_name = m_registry.get().find(parameter.get_pointee_name());
            if (!type_name.has_value()) {
                return std::unexpected(
                    std::format("Type {} not found in registry", parameter.get_pointee_name())
                );
            }

            m_writer +=
                helper::format_parameter(type_name.get_ponintee_type(), parameter.get_name());
        }
    }

    void write_call_arguments(std::span<const Parameter> parameters)
    {
        for (const Parameter& parameter: parameters) {
            m_writer += parameter.unwrap_argument(parameter) + ", ";
        }
    }

    std::string m_writer;
    std::string m_namespace_name;
    std::reference_wrapper<Register> m_registry;
};
} // namespace cc_abi_gen::emitter
