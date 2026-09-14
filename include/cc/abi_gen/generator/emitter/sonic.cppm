export module cc_abi_gen_generator:sonic_emitter;

import std;
import cc_abi_gen_parser;
import :helper_formater;

export namespace cc_abi_gen::generator::emitter {

class Sonic
{
public:
    Sonic() = default;

    Sonic(std::reference_wrapper<parser::Registry> registry, std::string_view namespace_name) :
        m_registry{registry},
        m_namespace_name{namespace_name}
    {
    }

    ~Sonic() = default;
    Sonic(const Sonic&) = delete;
    Sonic& operator=(const Sonic&) = delete;
    Sonic(Sonic&&) = default;
    Sonic& operator=(Sonic&&) = default;

    Sonic& add_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
        return *this;
    }

    Sonic& add_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
        return *this;
    }

    Sonic& add_registry(parser::Registry&& registry) noexcept
    {
        m_registry = std::move(registry);
        return *this;
    }

    std::expected<std::string, std::string> render(const parser::vtable::Model& model)
    {
        m_writer.clear();

        m_writer += helper::format_header(
            helper::GenTarget::Sonic,
            model.get_domain_name(),
            model.get_class_name(),
            m_namespace_name,
            model.get_struct_name()
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
            if (slot.is_destroy() || slot.is_get_name()) {
                continue;
            }

            auto method = write_method(slot);
            if (!method.has_value()) {
                return std::unexpected(method.error());
            }
        }

        m_writer += helper::format_footer(helper::GenTarget::Sonic, m_namespace_name);

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

    const std::string& get_writer() noexcept
    {
        return m_writer;
    }

    const std::string& get_namespace_name() noexcept
    {
        return m_namespace_name;
    }

    const parser::Registry& get_registry() noexcept
    {
        return m_registry;
    }

private:
    std::expected<void, std::string> write_method(const parser::slot::Slot& slot)
    {
        auto middle = slot.extract_parameters();

        m_writer += helper::format_method_signature(slot.get_name(), m_namespace_name);

        auto parameters_list = write_cpp_parameter_list(middle);
        if (!parameters_list.has_value()) {
            return parameters_list;
        }

        m_writer += helper::format_method_body_start(slot.get_name(), m_namespace_name);

        auto call_arguments = write_call_arguments(middle);
        if (!call_arguments.has_value()) {
            return call_arguments;
        }

        m_writer += helper::format_method_body_end();

        return {};
    }

    std::expected<void, std::string>
    write_cpp_parameter_list(std::span<const parser::helper::Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto model = m_registry.get().find(std::string{parameter.get_pointee_name()});
            if (!model.has_value()) {
                return std::unexpected(
                    std::format("Type {} not found in registry", parameter.get_pointee_name())
                );
            }

            m_writer += helper::format_parameter(
                model->get().get_pointee_type(m_namespace_name),
                parameter.get_name()
            );
        }

        return {};
    }

    std::expected<void, std::string>
    write_call_arguments(std::span<const parser::helper::Parameter> parameters)
    {
        for (const parser::helper::Parameter& parameter: parameters) {
            auto model = m_registry.get().find(std::string{parameter.get_pointee_name()});
            if (!model.has_value()) {
                return std::unexpected(
                    std::format("Type {} not found in registry", parameter.get_pointee_name())
                );
            }

            m_writer += model->get().unwrape_type(parameter.get_name()) + ", ";
        }

        return {};
    }

    std::string m_writer;
    std::string m_namespace_name;
    std::reference_wrapper<parser::Registry> m_registry;
};
} // namespace cc_abi_gen::generator::emitter
