module;

#include <stdio.h>
#include <string>

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

using PathCallback = std::function<void(std::filesystem::path& model)>;

struct PathComponents
{
    std::string tier;
    std::string domain;
    std::string mode;
    std::string file;
};

class Emitter
{
public:
    Emitter(const parser::Registry& registry, std::string&& base_folder, std::string&& name_space) :
        m_registry{registry},
        m_base_folder{std::move(base_folder)},
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

    Emitter& add_base_folder(std::string&& base_folder) noexcept
    {
        m_base_folder = std::move(base_folder);
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

    // Unified generate method - uses root for path resolution
    std::expected<void, std::string> generate(
        std::filesystem::path& root,
        std::string_view out_dir,
        const parser::vtable::Model& model,
        const Mode& mode
    )
    {
        if (mode == Mode::Both) {
            auto result = generate(root, out_dir, model, Mode::Builder);
            if (!result) {
                return result;
            }

            return generate(root, out_dir, model, Mode::Sonic);
        }

        auto base_path = root / out_dir;

        auto rendered = render(base_path, model, mode);
        if (!rendered) {
            return std::unexpected(rendered.error());
        }

        auto components = extract_path_components(model, mode);
        if (!components) {
            return std::unexpected(components.error());
        }

        auto path = build_output_path(base_path, *components);

        auto write_result = m_file_writer.write(*rendered, path, root);
        if (!write_result) {
            return write_result;
        }


        // Dynamically build the graph folder-by-folder explicitly from the model's structure
        track_partition(base_path, *components);

        return {};
    }

    std::expected<void, std::string>
    generate_base_modules(std::filesystem::path& root, std::string_view out_dir)
    {
        auto output_root = root / out_dir;

        // Traverse the naturally built directory graph and write a base.cppm and BUILD file for
        // EVERY node
        for (const auto& [dir_path, children]: m_partitions_by_folder) {
            std::string domain_name = dir_path.filename().string();
            std::string target_name = dir_path.parent_path().filename().string();

            auto base_path = dir_path / "base.cppm";

            std::string base_content;

            // Gracefully handle the absolute root node name mapping
            if (dir_path == output_root || target_name.empty()) {
                auto local_children = children;
                for (auto& child: local_children) {
                    child.insert(0, std::format("{}_", m_base_folder));
                }

                auto base_rendered =
                    helper::format_base_module(m_base_folder, "", "", "", local_children);
                if (!base_rendered) {
                    return std::unexpected(std::move(base_rendered.error()));
                }

                base_content = *base_rendered;
            } else if (domain_name == m_namespace_name) {
                auto local_children = children;
                for (auto& child: local_children) {
                    child.insert(0, std::format("{}_{}_", m_base_folder, m_namespace_name));
                }

                auto base_rendered = helper::format_base_module(
                    m_base_folder,
                    "",
                    "",
                    m_namespace_name,
                    local_children
                );
                if (!base_rendered) {
                    return std::unexpected(std::move(base_rendered.error()));
                }

                base_content = *base_rendered;
            } else if (target_name == m_namespace_name) {
                auto local_children = children;
                for (auto& child: local_children) {
                    child.insert(
                        0,
                        std::format("{}_{}_{}_", m_base_folder, m_namespace_name, domain_name)
                    );
                }

                auto base_rendered = helper::format_base_module(
                    m_base_folder,
                    domain_name,
                    "",
                    m_namespace_name,
                    local_children
                );
                if (!base_rendered) {
                    return std::unexpected(std::move(base_rendered.error()));
                }

                base_content = *base_rendered;
            } else {
                auto local_children = children;
                for (auto& child: local_children) {
                    child.insert(0, ":");
                }

                auto base_rendered = helper::format_base_module(
                    m_base_folder,
                    domain_name,
                    target_name,
                    m_namespace_name,
                    local_children
                );
                if (!base_rendered) {
                    return std::unexpected(std::move(base_rendered.error()));
                }

                base_content = *base_rendered;
            }


            auto write_result = m_file_writer.write(base_content, base_path, root);
            if (!write_result) {
                return write_result;
            }

            std::println("Generating base module for {}::{}", target_name, domain_name);

            // Generate BUILD file for this hierarchy level.
            auto build_rendered = helper::format_build_file(
                m_base_folder,
                target_name,
                domain_name,
                m_namespace_name,
                children,
                children
            );
            if (!build_rendered) {
                return std::unexpected(std::move(build_rendered.error()));
            }

            auto build_path = dir_path / "BUILD";
            auto build_write_result = m_file_writer.write(*build_rendered, build_path, root);
            if (!build_write_result) {
                return build_write_result;
            }
        }

        return {};
    }

    std::expected<bool, std::string> check(
        std::filesystem::path& root,
        std::string_view out_dir,
        const parser::vtable::Model& model,
        const Mode& mode
    )
    {
        if (mode == Mode::Both) {
            auto result = check(root, out_dir, model, Mode::Builder);
            if (!result) {
                return result;
            }

            if (!*result) {
                return false;
            }

            return check(root, out_dir, model, Mode::Sonic);
        }

        auto base_path = root / out_dir;

        auto rendered = render(base_path, model, mode);
        if (!rendered) {
            return std::unexpected{std::move(rendered.error())};
        }

        auto components = extract_path_components(model, mode);
        if (!components) {
            return std::unexpected(components.error());
        }

        auto real_path = build_output_path(base_path, *components);

        auto diff_result = m_file_writer.diff(*rendered, real_path, base_path);
        if (!diff_result) {
            return std::unexpected{std::move(diff_result.error())};
        }

        if (diff_result->get_identical()) {
            std::println(stderr, "[cc_abi_gen] up to date: {}", real_path.string());
        } else {
            std::println("--- {} differs ---", real_path.string());
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

    void set_base_folder(std::string&& base_folder) noexcept
    {
        m_base_folder = std::move(base_folder);
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

    const std::string& get_base_folder() noexcept
    {
        return m_base_folder;
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
    void track_partition(const std::filesystem::path& output_root, const PathComponents& components)
    {
        // Translate the strings directly into a path to leverage slash-by-slash iteration
        std::filesystem::path base_path =
            std::filesystem::path(components.tier) / components.domain / components.mode;

        if (m_path_callback) {
            m_path_callback(base_path);
        }

        std::filesystem::path current_node = output_root;

        // Traverse each slash component and register it in the parent's vector
        for (const auto& part: base_path) {
            std::string child_name = part.string();
            auto& children = m_partitions_by_folder[current_node];

            if (std::ranges::find(children, child_name) == children.end()) {
                children.push_back(child_name);
            }

            current_node /= part; // Step into the next folder level
        }

        // At the leaf node, register the actual file (partition)
        std::string leaf_str = components.file;
        auto& leaf_children = m_partitions_by_folder[current_node];
        if (std::ranges::find(leaf_children, leaf_str) == leaf_children.end()) {
            leaf_children.push_back(std::move(leaf_str));
        }
    }

    std::expected<std::string, std::string>
    render(std::filesystem::path& root, const parser::vtable::Model& model, const Mode& mode)
    {
        m_writer.clear();

        auto partition = model.to_file_name();
        if (!partition.has_value()) {
            return std::unexpected("Failed to get partition name");
        }

        auto header_result = helper::format_header(
            to_gen_target(mode),
            m_base_folder,
            model.get_domain_name(),
            model.get_header_path(),
            model.get_class_name(),
            m_namespace_name,
            root,
            model.get_struct_name(),
            *partition
        );
        if (!header_result) {
            return std::unexpected(header_result.error());
        }

        m_writer += *header_result;

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
            auto get_name_result = helper::format_get_name_decl(m_namespace_name, root);
            if (!get_name_result) {
                return std::unexpected(get_name_result.error());
            }

            m_writer += *get_name_result;
        } else if (mode == Mode::Builder) {
            auto accessor = write_vtable_accessor(root, model, mode);
            if (!accessor.has_value()) {
                return std::unexpected(accessor.error());
            }
        } else {
            return std::unexpected(std::format("Invalid mode for render function: {}", mode));
        }

        auto footer_result = helper::format_footer(to_gen_target(mode), root);
        if (!footer_result) {
            return std::unexpected(footer_result.error());
        }
        m_writer += *footer_result;

        return m_writer;
    }

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

    std::expected<PathComponents, std::string>
    extract_path_components(const parser::vtable::Model& model, const Mode& mode) const
    {
        auto tier = model.to_tier();
        if (!tier.has_value()) {
            return std::unexpected{tier.error()};
        }

        const auto domain = model.get_domain_name();
        auto file = model.to_file_name();
        if (!file.has_value()) {
            return std::unexpected{std::move(file.error())};
        }

        std::string mode_str;
        if (mode == emitter::Mode::Sonic) {
            mode_str = "sonic";
        } else if (mode == emitter::Mode::Builder) {
            mode_str = "builder";
        } else {
            return std::unexpected{
                std::format("Mode not supported in extract_path_components `{}`", mode)
            };
        }

        return PathComponents{
            .tier = *std::move(tier),
            .domain = std::string{domain},
            .mode = std::move(mode_str),
            .file = *std::move(file)
        };
    }

    std::filesystem::path
    build_output_path(const std::filesystem::path& root, const PathComponents& components) const
    {
        auto calculated_path = root / components.tier / components.domain / components.mode /
                               (components.file + ".cppm");

        if (m_path_callback) {
            m_path_callback(calculated_path);
        }
        return calculated_path;
    }

    std::expected<void, std::string>
    write_method(std::filesystem::path& root, const parser::slot::Slot& slot, const Mode& mode)
    {
        auto ms_result = helper::format_method_signature(
            slot.get_name(),
            m_namespace_name,
            mode == Mode::Builder
        );
        if (!ms_result) {
            return std::unexpected(ms_result.error());
        }
        m_writer += *ms_result;

        auto parameters_list = write_cpp_parameter_list(slot.extract_parameters());
        if (!parameters_list.has_value()) {
            return parameters_list;
        }

        if (mode == Mode::Builder) {
            auto vme_result = helper::format_virtual_method_end(root);
            m_writer += vme_result;
        } else if (mode == Mode::Sonic) {
            auto mbs_result =
                helper::format_method_body_start(slot.get_name(), m_namespace_name, root);
            if (!mbs_result) {
                return std::unexpected(mbs_result.error());
            }
            m_writer += *mbs_result;

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
                auto param_result =
                    helper::format_parameter(parameter.get_type(), parameter.get_name());
                if (!param_result) {
                    return std::unexpected(param_result.error());
                }
                m_writer += *param_result;
                continue;
            }

            auto param_result = helper::format_parameter(
                model->get().to_pointee_type(m_namespace_name),
                parameter.get_name()
            );
            if (!param_result) {
                return std::unexpected(param_result.error());
            }
            m_writer += *param_result;
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
        auto vtas_result = helper::format_vtable_accessor_start(
            model.get_struct_name(),
            model.get_struct_size_macro(),
            root
        );
        if (!vtas_result) {
            return std::unexpected(vtas_result.error());
        }
        m_writer += *vtas_result;

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
            auto vtfd_result =
                helper::format_vtable_field_destroy(slot.get_name(), model.get_class_name(), root);
            if (!vtfd_result) {
                return std::unexpected(vtfd_result.error());
            }
            m_writer += *vtfd_result;

            return {};
        }

        if (slot.is_get_name()) {
            auto vtfg_result =
                helper::format_vtable_field_get_name(slot.get_name(), model.get_class_name(), root);
            if (!vtfg_result) {
                return std::unexpected(vtfg_result.error());
            }
            m_writer += *vtfg_result;

            return {};
        }

        auto vtfgs_result = helper::format_vtable_field_generic_start(slot.get_name());
        if (!vtfgs_result) {
            return std::unexpected(vtfgs_result.error());
        }
        m_writer += *vtfgs_result;

        write_c_parameter_list(slot.get_parameters());

        auto vtfgm_result = helper::format_vtable_field_generic_middle(
            model.get_class_name(),
            slot.get_name(),
            self_parameter_name(slot),
            root
        );
        if (!vtfgm_result) {
            return std::unexpected(vtfgm_result.error());
        }
        m_writer += *vtfgm_result;

        auto call_arguments = write_call_arguments(slot, mode);
        if (!call_arguments.has_value()) {
            return call_arguments;
        }

        auto vtfge_result =
            helper::format_vtable_field_generic_end(failable_parameter_name(slot), root);
        if (!vtfge_result) {
            return std::unexpected(vtfge_result.error());
        }
        m_writer += *vtfge_result;

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

            auto param_result =
                helper::format_parameter(parameter.get_type(), parameter.get_name());
            if (!param_result) {
                // This is a void function, but format_parameter returns expected
                // In practice this shouldn't fail for simple parameters
                m_writer += "/* error */";
                continue;
            }
            m_writer += *param_result;
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
    std::string m_base_folder;
    writer::Writer m_file_writer;
    std::reference_wrapper<const parser::Registry> m_registry;
    PathCallback m_path_callback;
    std::map<std::filesystem::path, std::vector<std::string>> m_partitions_by_folder;
};

} // namespace cc_abi_gen::generator::emitter
