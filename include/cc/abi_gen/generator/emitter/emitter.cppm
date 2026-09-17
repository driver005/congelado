module;

#include <stdio.h>

export module cc_abi_gen_generator:emitter;

import std;
import cc_abi_gen_parser;
import cc_abi_gen_writer;
import :helper_formater;

export namespace cc_abi_gen::generator::emitter {

enum class Mode
{
    Builder,
    Sonic,
    Both
};

constexpr inline std::optional<Mode> from_string(std::string_view str) noexcept
{
    if (str == "builder") {
        return Mode::Builder;
    }
    if (str == "sonic") {
        return Mode::Sonic;
    }
    if (str == "both") {
        return Mode::Both;
    }
    return std::nullopt;
}

} // namespace cc_abi_gen::generator::emitter

template<>
struct std::formatter<cc_abi_gen::generator::emitter::Mode> : std::formatter<std::string_view>
{
    auto format(cc_abi_gen::generator::emitter::Mode mode, std::format_context& ctx) const
    {
        // C++20: brings the enum values into the local scope
        using enum cc_abi_gen::generator::emitter::Mode;

        std::string_view name;
        switch (mode) {
            case Builder:
                name = "Builder";
                break;
            case Sonic:
                name = "Sonic";
                break;
            case Both:
                name = "Both";
                break;
            default:
                name = "Unknown";
                break;
        }

        return std::formatter<std::string_view>::format(name, ctx);
    }
};

export namespace cc_abi_gen::generator::emitter {

using PathCallback =
    std::function<std::filesystem::path(const parser::vtable::Model& model, Mode mode)>;

class Emitter
{
public:
    Emitter(const parser::Registry& registry, std::string&& name_space) :
        m_registry{registry},
        m_namespace_name{std::move(name_space)}
    {
    }

    ~Emitter() = default;
    Emitter(const Emitter&) = delete;
    Emitter& operator=(const Emitter&) = delete;
    Emitter(Emitter&&) = default;
    Emitter& operator=(Emitter&&) = default;

    Emitter& add_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
        return *this;
    }

    Emitter& add_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
        return *this;
    }

    Emitter& add_file_writer(writer::Writer&& file_writer) noexcept
    {
        m_file_writer = std::move(file_writer);
        return *this;
    }

    Emitter& add_registry(const parser::Registry& registry) noexcept
    {
        m_registry = registry;
        return *this;
    }

    Emitter& add_path_callback(PathCallback&& path_callback) noexcept
    {
        m_path_callback = std::move(path_callback);
        return *this;
    }

    std::expected<std::string, std::string>
    render(std::filesystem::path& root, const parser::vtable::Model& model, const Mode& mode)
    {
        m_writer.clear();

        m_writer += helper::format_header(
            to_gen_target(mode),
            model.get_domain_name(),
            model.get_header_path(),
            model.get_class_name(),
            m_namespace_name,
            root,
            model.get_struct_name()
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
            if (slot.is_destroy() || slot.is_get_name()) {
                continue;
            }

            auto method = write_method(root, slot, mode);
            if (!method.has_value()) {
                return std::unexpected(method.error());
            }
        }

        if (mode == Mode::Sonic) {
            m_writer += helper::format_get_name_decl(m_namespace_name, root);
        } else if (mode == Mode::Builder) {
            auto accessor = write_vtable_accessor(root, model, mode);
            if (!accessor.has_value()) {
                return std::unexpected(accessor.error());
            }
        } else {
            return std::unexpected(std::format("Invalid mode for render function: {}", mode));
        }

        m_writer += helper::format_footer(to_gen_target(mode), m_namespace_name, root);

        return m_writer;
    }

    // Unified generate method - uses root for path resolution
    std::expected<void, std::string>
    generate(std::filesystem::path& root, const parser::vtable::Model& model, const Mode& mode)
    {
        if (mode == Mode::Both) {
            auto result = generate(root, model, Mode::Builder);
            if (!result) {
                return result;
            }

            return generate(root, model, Mode::Sonic);
        }

        auto rendered = render(root, model, mode);
        if (!rendered) {
            return std::unexpected(rendered.error());
        }

        auto path = resolve_output_path(root, model, mode);
        if (!path) {
            return std::unexpected(path.error());
        }

        return m_file_writer.write(*rendered, *path, root);
    }

    std::expected<bool, std::string>
    check(std::filesystem::path& root, const parser::vtable::Model& model, const Mode& mode)
    {
        if (mode == Mode::Both) {
            auto result = check(root, model, Mode::Builder);
            if (!result) {
                return result;
            }

            if (!*result) {
                return false;
            }

            return check(root, model, Mode::Sonic);
        }


        auto rendered = render(root, model, mode);
        if (!rendered) {
            return std::unexpected{std::move(rendered.error())};
        }

        auto real_path = resolve_output_path(root, model, mode);
        if (!real_path.has_value()) {
            return std::unexpected{std::move(real_path.error())};
        }
        auto diff_result = m_file_writer.diff(*rendered, *real_path, root);
        if (!diff_result) {
            return std::unexpected{std::move(diff_result.error())};
        }

        if (diff_result->get_identical()) {
            std::println(stderr, "[cc_abi_gen] up to date: {}", real_path->string());
        } else {
            std::println("--- {} differs ---", real_path->string());
            std::print("{}", diff_result->get_unified_diff());
            return false;
        }

        return true;
    }

    void set_path_callback(PathCallback&& path_callback) noexcept
    {
        m_path_callback = std::move(path_callback);
    }

    void set_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
    }

    void set_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
    }

    void set_file_writer(writer::Writer&& file_writer) noexcept
    {
        m_file_writer = std::move(file_writer);
    }

    void set_registry(const parser::Registry& registry)
    {
        m_registry = std::cref(registry);
    }

    const std::string& get_writer() noexcept
    {
        return m_writer;
    }

    const std::string& get_namespace_name() noexcept
    {
        return m_namespace_name;
    }

    const writer::Writer& get_file_writer() noexcept
    {
        return m_file_writer;
    }

    const parser::Registry& get_registry() noexcept
    {
        return m_registry;
    }

    const PathCallback& get_output_path_callback() const noexcept
    {
        return m_path_callback;
    }

private:
    static helper::GenTarget to_gen_target(Mode mode) noexcept
    {
        switch (mode) {
            case Mode::Builder:
                return helper::GenTarget::Builder;
            case Mode::Sonic:
                return helper::GenTarget::Sonic;
            case Mode::Both:
                return helper::GenTarget::Builder; // unused for Both
        }
    }

    std::expected<std::filesystem::path, std::string> resolve_output_path(
        std::filesystem::path& root,
        const parser::vtable::Model& model,
        const Mode& mode
    ) const
    {
        auto tier = model.to_tier();
        if (!tier.has_value()) {
            return std::unexpected{tier.value()};
        }

        const auto domain = model.get_domain_name();
        auto file = model.to_file_name();
        if (!file.has_value()) {
            return std::unexpected{std::move(file.value())};
        }

        std::string mode_str;
        if (mode == emitter::Mode::Sonic) {
            mode_str = "sonic";
        } else if (mode == emitter::Mode::Builder) {
            mode_str = "builder";
        } else {
            return std::unexpected{
                std::format("Mode not supported in resolve_output_path `{}`", mode)
            };
        }

        return root / *tier / domain / mode_str / (*file + ".cppm");
    }

    std::expected<void, std::string>
    write_method(std::filesystem::path& root, const parser::slot::Slot& slot, const Mode& mode)
    {
        m_writer += helper::format_method_signature(
            slot.get_name(),
            m_namespace_name,
            mode == Mode::Builder
        );

        auto parameters_list = write_cpp_parameter_list(slot.extract_parameters());
        if (!parameters_list.has_value()) {
            return parameters_list;
        }

        if (mode == Mode::Builder) {
            m_writer += helper::format_virtual_method_end(root);
        } else if (mode == Mode::Sonic) {
            m_writer += helper::format_method_body_start(slot.get_name(), m_namespace_name, root);

            auto call_arguments = write_call_arguments(slot, mode);
            if (!call_arguments.has_value()) {
                return call_arguments;
            }

            m_writer += helper::format_method_body_end(root);
        } else {
            return std::unexpected(std::format("Invalid mode for write_method function: {}", mode));
        }

        return {};
    }

    std::expected<void, std::string>
    write_cpp_parameter_list(std::span<const parser::helper::Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto model = parameter.has_pointee()
                             ? m_registry.get().find(parameter.get_registry_key())
                             : std::nullopt;

            if (!model.has_value()) {
                m_writer += helper::format_parameter(parameter.get_type(), parameter.get_name());
                continue;
            }

            m_writer += helper::format_parameter(
                model->get().to_pointee_type(m_namespace_name),
                parameter.get_name()
            );
        }

        return {};
    }

    std::expected<void, std::string>
    write_call_arguments(const parser::slot::Slot& slot, const Mode& mode)
    {
        auto middle = slot.extract_parameters();

        for (auto&& [index, parameter]: middle | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto model = parameter.has_pointee()
                             ? m_registry.get().find(parameter.get_registry_key())
                             : std::nullopt;

            if (!model.has_value()) {
                m_writer += parameter.get_name();
                continue;
            }

            if (mode == Mode::Builder) {
                m_writer += model->get().wrape_type(m_namespace_name, parameter.get_name());
            } else if (mode == Mode::Sonic) {
                m_writer += model->get().unwrape_type(parameter.get_name());
            } else {
                return std::unexpected(
                    std::format("Invalid mode for write_call_arguments function: {}", mode)
                );
            }
        }

        return {};
    }

    std::expected<void, std::string> write_vtable_accessor(
        std::filesystem::path& root,
        const parser::vtable::Model& model,
        const Mode& mode
    )
    {
        m_writer += helper::format_vtable_accessor_start(
            model.get_struct_name(),
            model.get_struct_size_macro(),
            root
        );

        for (const parser::slot::Slot& slot: model.get_slots()) {
            auto field = write_vtable_field(root, model, slot, mode);
            if (!field.has_value()) {
                return field;
            }
        }

        m_writer += helper::format_vtable_accessor_end(root);

        return {};
    }

    std::expected<void, std::string> write_vtable_field(
        std::filesystem::path& root,
        const parser::vtable::Model& model,
        const parser::slot::Slot& slot,
        const Mode& mode
    )
    {
        if (slot.is_destroy()) {
            m_writer +=
                helper::format_vtable_field_destroy(slot.get_name(), model.get_class_name(), root);

            return {};
        }

        if (slot.is_get_name()) {
            m_writer +=
                helper::format_vtable_field_get_name(slot.get_name(), model.get_class_name(), root);

            return {};
        }

        m_writer += helper::format_vtable_field_generic_start(slot.get_name());

        write_c_parameter_list(slot.get_parameters());

        m_writer += helper::format_vtable_field_generic_middle(
            model.get_class_name(),
            slot.get_name(),
            self_parameter_name(slot),
            root
        );

        auto call_arguments = write_call_arguments(slot, mode);
        if (!call_arguments.has_value()) {
            return call_arguments;
        }

        m_writer += helper::format_vtable_field_generic_end(failable_parameter_name(slot), root);

        return {};
    }

    std::string_view self_parameter_name(const parser::slot::Slot& slot) const noexcept
    {
        auto parameters = slot.get_parameters();
        return parameters.empty() ? std::string_view{"plugin_context"}
                                  : parameters.front().get_name();
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
    writer::Writer m_file_writer;
    std::reference_wrapper<const parser::Registry> m_registry;
    PathCallback m_path_callback;
};

} // namespace cc_abi_gen::generator::emitter
