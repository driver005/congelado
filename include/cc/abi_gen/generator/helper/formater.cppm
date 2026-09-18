module;

#include <cstdio>
#include <nlohmann/json.hpp>

export module cc_abi_gen_generator:helper_formater;

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
    std::string_view domain_name,        // actual domain (e.g., "cache")
    std::string_view header_path,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root,
    std::string_view c_struct_name,
    std::string_view mode_name           // "builder" or "sonic"
)
{
    std::string_view target_name = domain_name;  // actual domain for module name pos 2
    std::string_view extra_imports = "";
    std::string_view inheritance = "";
    std::string class_body;

    if (target == GenTarget::Builder) {
        auto body_result = cc::templating::TemplateRenderer::render_template(
            "class_body_builder",
            {{"class_name", std::string{cc_class_name}}, {"extra_ctor", ""}}
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
    std::string_view domain_name,       // actual domain (cache, logger)
    std::string_view header_path,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    const std::filesystem::path& repo_root,
    std::string_view c_struct_name = "",
    std::string_view partition = ""
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
        mode_name
    );

    return cc::templating::TemplateRenderer::render_template(
        "module_header",
        {{"domain_name", std::string{config.mode_name}},   // builder/sonic for namespace
         {"header_path", config.header_path},
         {"extra_includes", config.extra_includes},
         {"target_name", config.target_name},              // actual domain for module name
         {"extra_imports", config.extra_imports},
         {"extra_pre_class", ""},
         {"class_name", std::string{cc_class_name}},
         {"inheritance", config.inheritance},
         {"class_body", config.class_body},
         {"namespace_name", std::string{namespace_name}},
         {"partition", std::string{partition}}}
    );
}

inline std::expected<std::string, std::string> format_footer(
    GenTarget target,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    std::string_view mode_name = (target == GenTarget::Builder) ? "builder" : "sonic";

    auto extra_methods_result = cc::templating::TemplateRenderer::render_template(
        "method_string_accessor_sonic",
        {{"namespace_name", std::string{mode_name}}, {"slot_name", "get_name"}}
    );
    if (!extra_methods_result) {
        return std::unexpected(extra_methods_result.error());
    }

    return cc::templating::TemplateRenderer::render_template(
        "module_footer",
        {{"extra_methods", *extra_methods_result},
         {"target_name", std::string{mode_name}},
         {"namespace_name", std::string{mode_name}},
         {"domain_name", std::string{mode_name}}}
    );
}

inline std::expected<std::string, std::string> format_parameter(std::string_view type, std::string_view name) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "parameter",
        {{"type", std::string{type}}, {"name", std::string{name}}}
    );
}

inline std::expected<std::string, std::string> format_method_signature(
    std::string_view method_name,
    std::string_view namespace_name,
    bool is_virtual = false
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_signature",
        {{"virtual_prefix", is_virtual ? "virtual " : ""},
         {"return_type", "void"},
         {"namespace_name", std::string{namespace_name}},
         {"method_name", std::string{method_name}}}
    );
}

inline std::expected<std::string, std::string> format_get_name_decl(
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_decl_string_accessor",
        {{"namespace_name", std::string{namespace_name}}, {"method_name", "get_name"}}
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
    return "\n                };\n\n                return &vtable;\n            }\n";
}

inline std::expected<std::string, std::string> format_vtable_field_destroy(
    std::string_view slot_name,
    std::string_view cc_class_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_destroy",
        {{"slot_name", std::string{slot_name}}, {"class_name", std::string{cc_class_name}}}
    );
}

inline std::expected<std::string, std::string> format_vtable_field_get_name(
    std::string_view slot_name,
    std::string_view cc_class_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_string_accessor",
        {{"slot_name", std::string{slot_name}},
         {"class_name", std::string{cc_class_name}},
         {"param_type", "TF_String*"},
         {"param_name", "out"}}
    );
}

inline std::expected<std::string, std::string> format_vtable_field_generic_start(std::string_view slot_name) noexcept
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
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
) noexcept
{
    return cc::templating::TemplateRenderer::render_template(
        "method_body_start",
        {{"namespace_name", std::string{namespace_name}},
         {"method_name", std::string{method_name}},
         {"result_prefix", ""}}
    );
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
    std::string_view domain_name,
    std::string_view target_name,
    std::string_view header_path,
    std::string_view extra_includes,
    std::string_view extra_imports,
    std::string_view namespace_name,
    const std::vector<std::string>& partitions
) noexcept
{
    nlohmann::json data;
    data["domain_name"] = std::string{domain_name};
    data["target_name"] = std::string{target_name};
    data["header_path"] = std::string{header_path};
    data["extra_includes"] = std::string{extra_includes};
    data["extra_imports"] = std::string{extra_imports};
    data["namespace_name"] = std::string{namespace_name};
    data["partitions"] = partitions;

    return cc::templating::TemplateRenderer::render_template_json("module_base", data);
}

} // namespace cc_abi_gen::helper
