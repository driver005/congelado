export module cc_abi_gen_generator:builder_emitter;

import std;
import cc_abi_gen_parser;
import :helper_formater;

export namespace cc_abi_gen::generator::emitter {

class Builder
{
public:
    Builder() = default;

    Builder(std::reference_wrapper<parser::Registry> registry, std::string_view namespace_name) :
        m_registry{registry},
        m_namespace_name{namespace_name}
    {
    }

    ~Builder() = default;
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;
    Builder(Builder&&) = default;
    Builder& operator=(Builder&&) = default;

    Builder& add_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
        return *this;
    }

    Builder& add_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
        return *this;
    }

    Builder& add_registry(parser::Registry&& registry) noexcept
    {
        m_registry = std::move(registry);
        return *this;
    }

    std::string render(const parser::vtable::Model& model)
    {
        m_writer.clear();

        m_writer += helper::format_header(
            helper::GenTarget::Builder,
            model.get_domain_name(),
            model.get_class_name(),
            m_namespace_name,
            model.get_struct_name()
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
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

    void set_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
    }

    void set_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
    }

    void set_registry(parser::Registry&& registry) noexcept
    {
        m_namespace_name = std::move(registry);
    }

    const std::string& get_writer() const noexcept
    {
        return m_writer;
    }

    const std::string& get_namespace_name() const noexcept
    {
        return m_namespace_name;
    }

    const parser::Registry& get_registry() const noexcept
    {
        return m_registry;
    }

private:
    void write_virtual_method(const parser::slot::Slot& slot)
    {
        m_writer += helper::format_method_signature(slot.get_name(), m_namespace_name);

        write_cpp_parameter_list(slot.extract_parameters());

        m_writer += helper::format_virtual_method_end();
    }

    std::expected<void, std::string>
    write_cpp_parameter_list(std::span<const parser::helper::Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto type_name = m_registry.get().find(std::string{parameter.get_pointee_name()});
            if (!type_name.has_value()) {
                return std::unexpected(
                    std::format("Type {} not found in registry", parameter.get_pointee_name())
                );
            }

            m_writer += helper::format_parameter(
                type_name->get().get_pointee_type(m_namespace_name),
                parameter.get_name()
            );
        }
    }

    void write_vtable_accessor(const parser::vtable::Model& model)
    {
        m_writer += helper::format_vtable_accessor_start(
            model.get_struct_name(),
            model.get_struct_size_macro()
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
            write_vtable_field(model, slot);
        }

        m_writer += helper::format_vtable_accessor_end();
    }

    void write_vtable_field(const parser::vtable::Model& model, const parser::slot::Slot& slot)
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

    void write_c_parameter_list(std::span<const parser::helper::Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            m_writer += helper::format_parameter(parameter.get_type(), parameter.get_name());
        }
    }

    std::expected<void, std::string> write_call_arguments(const parser::slot::Slot& slot)
    {
        auto middle = slot.extract_parameters();

        for (auto&& [index, parameter]: middle | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto model = m_registry.get().find(std::string{parameter.get_pointee_name()});
            if (!model.has_value()) {
                return std::unexpected(
                    std::format("Type {} not found in registry", parameter.get_pointee_name())
                );
            }

            m_writer += model->get().wrape_type(m_namespace_name, parameter.get_name());
        }

        return {};
    }

    std::string_view failable_parameter_name(const parser::slot::Slot& slot)
    {
        auto failable = slot.extract_failable();

        if (failable.has_value()) {
            return failable->get().get_name();
        }

        return "status";
    }

    std::string m_writer;
    std::string m_namespace_name;
    std::reference_wrapper<parser::Registry> m_registry;
};
} // namespace cc_abi_gen::generator::emitter
