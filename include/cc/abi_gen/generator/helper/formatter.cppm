module;

#include <cstdio>
#include <nlohmann/json.hpp>

export module cc_abi_gen_generator:helper_formatter;

import std;
import cc_templating;
import :helper_dependency_info;

export namespace cc_abi_gen::helper {

enum class GenTarget
{
    Builder,
    Sonic
};

struct HeaderConfig
{
    std::string target_name;
    std::string header_path;
    std::string extra_includes;
    std::string extra_imports;
    std::string inheritance;
    std::string class_body;
    std::string mode_name;
};

inline nlohmann::json dependencies_json(std::span<const DependencyInfo> dependencies)
{
    nlohmann::json array = nlohmann::json::array();
    for (const DependencyInfo& dependency: dependencies) {
        array.push_back(
            {{"type", dependency.get_sonic_type()},
             {"struct", dependency.get_struct_name()},
             {"member", dependency.get_member_name()},
             {"handle", dependency.get_handle_name()}}
        );
    }

    return array;
}

inline HeaderConfig build_header_config(
    GenTarget target,
    std::string_view domain_name, // actual domain (e.g., "cache")
    std::string_view header_path,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root,
    std::string_view c_struct_name,
    std::string_view mode_name, // "builder" or "sonic"
    std::string_view c_handle_name,
    std::string_view imports,
    std::span<const DependencyInfo> dependencies,
    std::string_view string_type,
    std::string_view includes,
    std::string_view registry_struct_name,
    std::string_view registry_handle_name
)
{
    std::string_view target_name = domain_name; // actual domain for module name pos 2
    std::string inheritance = "";
    std::string class_body;

    nlohmann::json data;
    data["class_name"] = std::string{cc_class_name};
    data["struct_name"] = std::string{c_struct_name};
    data["handle_name"] = std::string{c_handle_name};
    data["string_type"] = std::string{string_type};
    data["registry_struct"] = std::string{registry_struct_name};
    data["registry_handle"] = std::string{registry_handle_name};
    data["deps"] = dependencies_json(dependencies);

    if (target == GenTarget::Builder) {
        auto body_result =
            cc::templating::TemplateRenderer::render_template_json("class_body_builder", data);
        if (!body_result) {
            return {}; // will be handled by caller
        }
        class_body = std::move(*body_result);
    } else {
        auto inh_result = cc::templating::TemplateRenderer::render_template(
            "inheritance_sonic",
            {{"namespace_name", std::string{namespace_name}},
             {"class_name", std::string{cc_class_name}},
             {"struct_name", std::string{c_struct_name}},
             {"handle_name", std::string{c_handle_name}}}
        );
        if (!inh_result) {
            return {};
        }
        inheritance = std::move(*inh_result);
        auto body_result =
            cc::templating::TemplateRenderer::render_template_json("class_body_sonic", data);
        if (!body_result) {
            return {};
        }
        class_body = std::move(*body_result);
    }

    return {
        .target_name = std::string{target_name},
        .header_path = std::string{header_path},
        .extra_includes = std::string{includes},
        .extra_imports = std::string{imports},
        .inheritance = std::string{inheritance},
        .class_body = std::move(class_body),
        .mode_name = std::string{mode_name}
    };
}

inline std::expected<std::string, std::string> format_header(
    GenTarget target,
    std::string_view folder_name,
    std::string_view domain_name, // actual domain (cache, logger)
    std::string_view header_path,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    const std::filesystem::path& repo_root,
    std::string_view module_name,
    std::string_view c_struct_name,
    std::string_view partition,
    std::string_view c_handle_name,
    std::string_view imports,
    std::span<const DependencyInfo> dependencies,
    std::string_view string_type,
    std::string_view includes,
    std::string_view registry_struct_name,
    std::string_view registry_handle_name
) noexcept
{
    std::string_view mode_name = (target == GenTarget::Builder) ? "builder" : "sonic";

    const HeaderConfig config = build_header_config(
        target,
        domain_name,
        header_path,
        cc_class_name,
        namespace_name,
        repo_root,
        c_struct_name,
        mode_name,
        c_handle_name,
        imports,
        dependencies,
        string_type,
        includes,
        registry_struct_name,
        registry_handle_name
    );

    return cc::templating::TemplateRenderer::render_template(
        "module_header",
        {{"folder_name", std::string{folder_name}},
         {"domain_name", std::string{config.mode_name}}, // builder/sonic for namespace
         {"header_path", config.header_path},
         {"extra_includes", config.extra_includes},
         {"target_name", config.target_name}, // actual domain for module name
         {"extra_imports", config.extra_imports},
         {"extra_pre_class", ""},
         {"class_name", std::string{cc_class_name}},
         {"inheritance", config.inheritance},
         {"class_body", config.class_body},
         {"namespace_name", std::string{namespace_name}},
         {"module_name", std::string{module_name}},
         {"partition", std::string{partition}}}
    );
}

inline std::expected<std::string, std::string> format_footer(
    GenTarget target,
    std::string_view c_struct_name,
    std::string_view c_handle_name,
    std::span<const DependencyInfo> dependencies,
    [[maybe_unused]] const std::filesystem::path& repo_root,
    std::string_view string_type,
    std::string_view registry_struct_name,
    std::string_view registry_handle_name
) noexcept
{
    std::string_view mode_name = (target == GenTarget::Builder) ? "builder" : "sonic";

    std::string extra_members;
    if (target == GenTarget::Builder) {
        nlohmann::json data;
        data["struct_name"] = std::string{c_struct_name};
        data["handle_name"] = std::string{c_handle_name};
        data["deps"] = dependencies_json(dependencies);
        data["string_type"] = std::string{string_type};
        data["registry_struct"] = std::string{registry_struct_name};
        data["registry_handle"] = std::string{registry_handle_name};

        auto rendered =
            cc::templating::TemplateRenderer::render_template_json("builder_footer", data);
        if (!rendered) {
            return std::unexpected(std::move(rendered.error()));
        }
        extra_members = std::move(*rendered);
    }

    return cc::templating::TemplateRenderer::render_template(
        "module_footer",
        {{"extra_methods", ""},
         {"extra_members", std::move(extra_members)},
         {"target_name", std::string{mode_name}},
         {"namespace_name", std::string{mode_name}},
         {"domain_name", std::string{mode_name}}}
    );
}

inline std::expected<std::string, std::string>
format_parameter(std::string_view type, std::string_view name) noexcept
{
    constexpr std::string_view pointer_marker = "(*)";

    if (auto position = type.find(pointer_marker); position != std::string_view::npos) {
        std::string declaration{type};
        declaration.insert(position + 2, name);

        return declaration;
    }

    return cc::templating::TemplateRenderer::render_template(
        "parameter",
        {{"type", std::string{type}}, {"name", std::string{name}}}
    );
}

inline std::expected<std::string, std::string>
format_method_signature(std::string_view method_name, bool is_virtual) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_signature",
        {{"virtual_prefix", is_virtual ? "virtual " : ""},
         {"method_name", std::string{method_name}}}
    );
}

inline std::string
format_virtual_method_end([[maybe_unused]] const std::filesystem::path& repo_root)
{
    return ") noexcept = 0;\n";
}

inline std::expected<std::string, std::string> format_vtable_accessor_start(
    std::string_view c_struct_name,
    std::string_view struct_size_macro,
    std::string_view c_handle_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_accessor_start",
        {{"struct_name", std::string{c_struct_name}},
         {"struct_size_macro", std::string{struct_size_macro}},
         {"handle_name", std::string{c_handle_name}}}
    );
}

inline std::string
format_vtable_accessor_end([[maybe_unused]] const std::filesystem::path& repo_root)
{
    return "\n                };\n            }\n";
}

inline std::expected<std::string, std::string>
format_vtable_field_generic_start(std::string_view slot_name) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_generic_start",
        {{"slot_name", std::string{slot_name}}}
    );
}

inline std::expected<std::string, std::string> format_vtable_field_middle(
    std::string_view cc_class_name,
    std::string_view slot_name,
    std::string_view self_param_name
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_middle",
        {{"class_name", std::string{cc_class_name}},
         {"slot_name", std::string{slot_name}},
         {"self_param_name", std::string{self_param_name}}}
    );
}

inline std::string format_vtable_field_end()
{
    return "\n                );\n            },\n";
}

inline std::expected<std::string, std::string>
format_method_body_start(std::string_view method_name) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_body_start",
        {{"method_name", std::string{method_name}}}
    );
}

inline std::string format_method_body_end()
{
    return ");\n            }\n";
}

inline std::expected<std::string, std::string> format_base_module(
    std::string_view folder_name,
    std::string_view module_name,
    const std::vector<std::string>& imports
) noexcept
{
    nlohmann::json data;
    data["folder_name"] = std::string{folder_name};
    data["module_name"] = std::string{module_name};
    data["imports"] = imports;

    return cc::templating::TemplateRenderer::render_template_json("module_base", data);
}

inline std::expected<std::string, std::string> format_build_file(
    std::string_view folder_name,
    std::string_view namespace_name,
    std::string_view module_name,
    const std::vector<std::string>& partitions,
    const std::vector<std::string>& deps
) noexcept
{
    nlohmann::json data;
    data["folder_name"] = std::string{folder_name};
    data["namespace_name"] = std::string{namespace_name};
    data["module_name"] = std::string{module_name};
    data["partitions"] = partitions;
    data["deps"] = deps;

    return cc::templating::TemplateRenderer::render_template_json("build_domain", data);
}

} // namespace cc_abi_gen::helper
