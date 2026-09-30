module;

#include <cstdio>
#include <nlohmann/json.hpp>

export module cc_abi_gen_generator:helper_formatter;

import std;
import cc_templating;

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

inline HeaderConfig build_header_config(
    GenTarget target,
    std::string_view domain_name, // actual domain (e.g., "cache")
    std::string_view header_path,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root,
    std::string_view c_struct_name,
    std::string_view mode_name, // "builder" or "sonic"
    std::string_view c_handle_name
)
{
    std::string_view target_name = domain_name; // actual domain for module name pos 2
    std::string extra_imports = "";
    std::string inheritance = "";
    std::string class_body;

    if (target == GenTarget::Builder) {
        std::string extra_ctor;
        if (!c_handle_name.empty()) {
            extra_ctor = std::format(
                "{0}() noexcept :\n    m_handle{{.plugin_data = this}}\n{{\n}}\n\n",
                cc_class_name
            );
        }
        extra_ctor += std::format(
            "{0}(const {0}&) = delete;\n{0}& operator=(const {0}&) = delete;\n",
            cc_class_name
        );

        auto body_result = cc::templating::TemplateRenderer::render_template(
            "class_body_builder",
            {{"class_name", std::string{cc_class_name}}, {"extra_ctor", std::move(extra_ctor)}}
        );
        if (!body_result) {
            return {}; // will be handled by caller
        }
        class_body = std::move(*body_result);
    } else {
        extra_imports = "import cc_abi_sonic_registration;\n";
        auto inh_result = cc::templating::TemplateRenderer::render_template(
            "inheritance_sonic",
            {{"namespace_name", std::string{namespace_name}},
             {"class_name", std::string{cc_class_name}},
             {"struct_name", std::string{c_struct_name}}}
        );
        if (!inh_result) {
            return {};
        }
        inheritance = std::move(*inh_result);
        auto body_result = cc::templating::TemplateRenderer::render_template(
            "class_body_sonic",
            {{"class_name", std::string{cc_class_name}},
             {"struct_name", std::string{c_struct_name}},
             {"target_name", std::string{domain_name}}}
        );
        if (!body_result) {
            return {};
        }
        class_body = std::move(*body_result);
    }

    return {
        .target_name = std::string{target_name},
        .header_path = std::string{header_path},
        .extra_imports = std::string{extra_imports},
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
    std::string_view c_struct_name = "",
    std::string_view partition = "",
    std::string_view c_handle_name = ""
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
        c_handle_name
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
    std::string_view cc_class_name,
    std::string_view c_struct_name,
    std::string_view c_handle_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    std::string_view mode_name = (target == GenTarget::Builder) ? "builder" : "sonic";

    std::string extra_methods;
    std::string extra_members;
    if (target == GenTarget::Builder) {
        extra_methods = std::format(
            "const ::{}& get_vtable() const noexcept\n{{\n    return m_vtable;\n}}\n",
            c_struct_name
        );
        extra_members = std::format("\nprivate:\n    ::{} m_vtable;\n", c_struct_name);
        if (!c_handle_name.empty()) {
            extra_methods += std::format(
                "\nconst {}& get_handle() const noexcept\n{{\n    return m_handle;\n}}\n",
                c_handle_name
            );
            extra_members += std::format("    {} m_handle;\n", c_handle_name);
        }
    }

    return cc::templating::TemplateRenderer::render_template(
        "module_footer",
        {{"extra_methods", std::move(extra_methods)},
         {"extra_members", std::move(extra_members)},
         {"target_name", std::string{mode_name}},
         {"namespace_name", std::string{mode_name}},
         {"domain_name", std::string{mode_name}}}
    );
}

inline std::expected<std::string, std::string>
format_parameter(std::string_view type, std::string_view name) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "parameter",
        {{"type", std::string{type}}, {"name", std::string{name}}}
    );
}

inline std::expected<std::string, std::string> format_method_signature(
    std::string_view method_name,
    std::string_view status_type,
    bool is_virtual,
    bool is_failable
) noexcept
{
    if (!is_failable) {
        return cc::templating::TemplateRenderer::render_template(
            "method_signature_void",
            {{"virtual_prefix", is_virtual ? "virtual " : ""},
             {"method_name", std::string{method_name}}}
        );
    }

    return cc::templating::TemplateRenderer::render_template(
        "method_signature",
        {{"virtual_prefix", is_virtual ? "virtual " : ""},
         {"return_type", "void"},
         {"status_type", std::string{status_type}},
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
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_accessor_start",
        {{"struct_name", std::string{c_struct_name}},
         {"struct_size_macro", std::string{struct_size_macro}}}
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

inline std::expected<std::string, std::string> format_vtable_field_generic_middle(
    std::string_view cc_class_name,
    std::string_view slot_name,
    std::string_view self_param_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_generic_middle",
        {{"class_name", std::string{cc_class_name}},
         {"slot_name", std::string{slot_name}},
         {"self_param_name", std::string{self_param_name}},
         {"trailing_return", ""}}
    );
}

inline std::expected<std::string, std::string> format_vtable_field_generic_end(
    std::string_view status_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_generic_end",
        {{"status_name", std::string{status_name}}, {"error_return", ""}, {"success_return", ""}}
    );
}

inline std::expected<std::string, std::string> format_method_body_start(
    std::string_view method_name,
    std::string_view status_type,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_body_start",
        {{"status_type", std::string{status_type}},
         {"method_name", std::string{method_name}},
         {"result_prefix", ""}}
    );
}

inline std::expected<std::string, std::string>
format_method_body_void_start(std::string_view method_name) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_body_void_start",
        {{"method_name", std::string{method_name}}}
    );
}

inline std::string format_method_body_void_end()
{
    return ");\n            }\n";
}

inline std::expected<std::string, std::string> format_vtable_field_void_middle(
    std::string_view cc_class_name,
    std::string_view slot_name,
    std::string_view self_param_name
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_void_middle",
        {{"class_name", std::string{cc_class_name}},
         {"slot_name", std::string{slot_name}},
         {"self_param_name", std::string{self_param_name}}}
    );
}

inline std::string format_vtable_field_void_end()
{
    return "\n                );\n            },\n";
}

inline std::string format_method_body_end([[maybe_unused]] const std::filesystem::path& repo_root)
{
    return "\n                status.get_handle());\n\n"
           "                if (!status.ok()) {\n"
           "                    return std::unexpected{status};\n"
           "                }\n"
           "                return {};\n"
           "            }\n";
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
