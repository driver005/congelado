export module cc_abi_gen_generator:builder_emitter;

import std;
import cc_abi_gen_parser;
import :helper_formater;

export namespace cc_abi_gen::generator::emitter {

class Builder
{
public:
    Builder(const parser::Registry& registry, std::filesystem::path repo_root) :
        m_registry{registry},
        m_repo_root{std::move(repo_root)}
    {
    }

    Builder(
        const parser::Registry& registry,
        std::string&& namespace_name,
        std::filesystem::path repo_root
    ) :
        m_registry{registry},
        m_namespace_name{std::move(namespace_name)},
        m_repo_root{std::move(repo_root)}
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

    std::expected<std::string, std::string> render(const parser::vtable::Model& model)
    {
        m_writer.clear();

        m_writer += helper::format_header(
            helper::GenTarget::Builder,
            model.get_domain_name(),
            model.get_class_name(),
            m_namespace_name,
            m_repo_root,
            model.get_struct_name()
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
            if (slot.is_destroy() || slot.is_get_name()) {
                continue;
            }

            auto method = write_virtual_method(slot);
            if (!method.has_value()) {
                return std::unexpected(method.error());
            }
        }

        m_writer += helper::format_get_name_decl(m_namespace_name, m_repo_root);

        auto accessor = write_vtable_accessor(model);
        if (!accessor.has_value()) {
            return std::unexpected(accessor.error());
        }

        m_writer += helper::format_footer(helper::GenTarget::Builder, m_namespace_name, m_repo_root);

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
    std::expected<void, std::string> write_virtual_method(const parser::slot::Slot& slot)
    {
        m_writer += helper::format_method_signature(slot.get_name(), m_namespace_name);

        auto parameters_list = write_cpp_parameter_list(slot.extract_parameters());
        if (!parameters_list.has_value()) {
            return parameters_list;
        }

        m_writer += helper::format_virtual_method_end(m_repo_root);

        return {};
    }

    std::expected<void, std::string>
    write_cpp_parameter_list(std::span<const parser::helper::Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            // Scalar/by-value parameters (int64_t, size_t, an enum passed by value, ...) have no
            // pointee — they carry no opaque handle to wrap/unwrap, so pass their raw type through
            // unchanged instead of looking them up in the registry.
            if (!parameter.is_handle()) {
                m_writer += helper::format_parameter(parameter.get_type(), parameter.get_name());
                continue;
            }

            auto type_name = m_registry.get().find(parameter.get_registry_key());
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

        return {};
    }

    std::expected<void, std::string> write_vtable_accessor(const parser::vtable::Model& model)
    {
        m_writer += helper::format_vtable_accessor_start(
            model.get_struct_name(),
            model.get_struct_size_macro(),
            m_repo_root
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
            auto field = write_vtable_field(model, slot);
            if (!field.has_value()) {
                return field;
            }
        }

        m_writer += helper::format_vtable_accessor_end(m_repo_root);

        return {};
    }

    std::expected<void, std::string>
    write_vtable_field(const parser::vtable::Model& model, const parser::slot::Slot& slot)
    {
        if (slot.is_destroy()) {
            m_writer += helper::format_vtable_field_destroy(
                slot.get_name(),
                model.get_class_name(),
                m_repo_root
            );

            return {};
        }

        if (slot.is_get_name()) {
            m_writer += helper::format_vtable_field_get_name(
                slot.get_name(),
                model.get_class_name(),
                m_repo_root
            );

            return {};
        }

        m_writer += helper::format_vtable_field_generic_start(slot.get_name());

        write_c_parameter_list(slot.get_parameters());

        m_writer += helper::format_vtable_field_generic_middle(
            model.get_class_name(),
            slot.get_name(),
            self_parameter_name(slot),
            m_repo_root
        );

        auto call_arguments = write_call_arguments(slot);
        if (!call_arguments.has_value()) {
            return call_arguments;
        }

        m_writer +=
            helper::format_vtable_field_generic_end(failable_parameter_name(slot), m_repo_root);

        return {};
    }

    // Position 0's declared name — "plugin_context" for a domain-level slot, or the
    // owned instance handle's name for an instance-level slot (which drops the
    // separate plugin_context param and takes only its own handle as self).
    std::string_view self_parameter_name(const parser::slot::Slot& slot) const noexcept
    {
        auto parameters = slot.get_parameters();
        return parameters.empty() ? std::string_view{"plugin_context"} : parameters.front().get_name();
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

            if (!parameter.is_handle()) {
                m_writer += parameter.get_name();
                continue;
            }

            auto model = m_registry.get().find(parameter.get_registry_key());
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
    std::reference_wrapper<const parser::Registry> m_registry;
    std::filesystem::path m_repo_root;
};
} // namespace cc_abi_gen::generator::emitter
