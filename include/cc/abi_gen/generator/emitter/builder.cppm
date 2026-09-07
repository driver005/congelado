module;
#include <expected>
#include <functional>
export module cc_abi_gen_generator:builder_emitter;

import std;
import cc_abi_gen_parser;
import :type_registry;
import :slot_classifier;
import :helper_formater;

export namespace cc_abi_gen::emitter {

class Builder
{
public:
    Builder(std::reference_wrapper<Register> registry, std::string_view namespace_name) :
        m_registry{registry},
        m_namespace_name{namespace_name}
    {
    }

    std::string render(const vtable::Model& model)
    {
        m_writer.clear();

        m_writer += helper::format_header(
            helper::GenTarget::Builder,
            model.get_domain_name(),
            model.get_class_name(),
            m_namespace_name,
            model.get_struct_name()
        );

        for (const vtable::Slot& slot: model.get_slots()) {
            if (slot.is_destroy() || slot.is_get_name()) {
                continue;
            }

            write_virtual_method(slot);
        }

        m_writer += helper::format_get_name_decl(m_namespace_name);

        write_vtable_accessor(model);

        m_writer += helper::format_footer(helper::GenTarget::Builder, m_namespace_name);

        return m_writer;
    }

private:
    void write_virtual_method(const VtableSlot& slot)
    {
        m_writer += helper::format_method_signature(slot.get_name(), m_namespace_name);

        write_cpp_parameter_list(slot.middle_parameters());

        m_writer += helper::format_virtual_method_end();
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

    void write_vtable_accessor(const vtable::Model& model)
    {
        m_writer += helper::format_vtable_accessor_start(
            model.get_struct_name(),
            model.get_struct_size_macro(),
        );

        for (const vtable::Slot& slot: model.get_slots()) {
            write_vtable_field(model, slot);
        }

        m_writer += helper::format_vtable_accessor_end();
    }

    void write_vtable_field(const vtable::Model& model, const vtable::Slot& slot)
    {
        if (slot.is_destroy()) {
            m_writer +=
                helper::format_vtable_field_destroy(slot.get_name(), model.get_class_name());

            return;
        }

        if (slot.is_get_name()) {
            m_writer +=
                helper::format_vtable_field_get_name(slot.get_name(), model.get_class_name());

            return;
        }

        m_writer += helper::format_vtable_field_generic_start(slot.get_name());

        write_c_parameter_list(slot.get_parameters());

        m_writer +=
            helper::format_vtable_field_generic_middle(model.get_class_name(), slot.get_name());

        write_call_arguments(slot);

        m_writer += helper::format_vtable_field_generic_end(failable_parameter_name(slot));
    }

    void write_c_parameter_list(std::span<const Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            m_writer +=
                helper::format_builder_parameter(parameter.get_type(), parameter.get_name());
        }
    }

    void write_call_arguments(const VtableSlot& slot)
    {
        auto middle = slot.middle_parameters(slot);

        for (auto&& [index, parameter]: middle | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            m_writer += parameter.wrap_argument();
        }
    }

    std::string failable_parameter_name(const vtable::Slot& slot)
    {
        auto failable = slot.extract_failable();

        if (failable.has_value()) {
            return failable.value().get_name();
        }

        return "status";
    }

    std::string m_writer;
    std::string m_namespace_name;
    std::reference_wrapper<Register> m_registry;
};
} // namespace cc_abi_gen::emitter
