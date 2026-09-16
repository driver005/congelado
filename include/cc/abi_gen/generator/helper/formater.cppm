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
    std::string extra_includes;
    std::string extra_imports;
    std::string inheritance;
    std::string class_body;
};

inline HeaderConfig build_header_config(
    GenTarget target,
    std::string_view domain_name,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root,
    std::string_view c_struct_name
)
{
    if (target == GenTarget::Builder) {
        return {
            .target_name = "builder",
            .extra_includes = "#include \"c/intern/tf_status.h\"\n"
                               "#include \"c/intern/tf_tstring.h\"\n",
            .extra_imports = "",
            .inheritance = "",
            .class_body = cc::templating::TemplateRenderer::render_template(
                "class_body_builder",
                {
                    {"class_name", std::string{cc_class_name}},
                    {"extra_ctor", ""}
                }
            )
        };
    }

    return {
        .target_name = "sonic",
        .extra_includes = "",
        .extra_imports = "import cc_abi_sonic_registration;\n",
        .inheritance = cc::templating::TemplateRenderer::render_template(
            "inheritance_sonic",
            {
                {"namespace_name", std::string{namespace_name}},
                {"class_name", std::string{cc_class_name}},
                {"struct_name", std::string{c_struct_name}}
            }
        ),
        .class_body = cc::templating::TemplateRenderer::render_template(
            "class_body_sonic",
            {
                {"class_name", std::string{cc_class_name}},
                {"struct_name", std::string{c_struct_name}},
                {"domain_name", std::string{domain_name}}
            }
        )
    };
}

inline std::string format_header(
    GenTarget target,
    std::string_view domain_name,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    const std::filesystem::path& repo_root,
    std::string_view c_struct_name = ""
)
{
    const HeaderConfig config =
        build_header_config(target, domain_name, cc_class_name, namespace_name, repo_root, c_struct_name);

    return cc::templating::TemplateRenderer::render_template(
        "module_header",
        {
            {"domain_name", std::string{domain_name}},
            {"extra_includes", config.extra_includes},
            {"target_name", config.target_name},
            {"extra_imports", config.extra_imports},
            {"extra_pre_class", ""},
            {"class_name", std::string{cc_class_name}},
            {"inheritance", config.inheritance},
            {"class_body", config.class_body},
            {"namespace_name", std::string{namespace_name}}
        }
    );
}

inline std::string format_footer(
    GenTarget target,
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    const std::string_view target_name = (target == GenTarget::Builder) ? "builder" : "sonic";

    const std::string extra_methods =
        (target == GenTarget::Builder)
            ? ""
            : cc::templating::TemplateRenderer::render_template(
                  "method_string_accessor_sonic",
                  {
                      {"namespace_name", std::string{namespace_name}},
                      {"slot_name", "get_name"}
                  }
              );

    return cc::templating::TemplateRenderer::render_template(
        "module_footer",
        {
            {"extra_methods", extra_methods},
            {"target_name", std::string{target_name}},
            {"namespace_name", std::string{namespace_name}}
        }
    );
}

inline std::string format_parameter(std::string_view type, std::string_view name)
{
    return cc::templating::TemplateRenderer::render_template(
        "parameter",
        {
            {"type", std::string{type}},
            {"name", std::string{name}}
        }
    );
}

inline std::string format_method_signature(
    std::string_view method_name,
    std::string_view namespace_name,
    bool is_virtual = false
)
{
    return cc::templating::TemplateRenderer::render_template(
        "method_signature",
        {
            {"virtual_prefix", is_virtual ? "virtual " : ""},
            {"return_type", "void"},
            {"namespace_name", std::string{namespace_name}},
            {"method_name", std::string{method_name}}
        }
    );
}

inline std::string format_get_name_decl(
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "method_decl_string_accessor",
        {
            {"namespace_name", std::string{namespace_name}},
            {"method_name", "get_name"}
        }
    );
}

inline std::string format_virtual_method_end([[maybe_unused]] const std::filesystem::path& repo_root)
{
    return ") noexcept = 0;\n";
}

inline std::string format_vtable_accessor_start(
    std::string_view c_struct_name,
    std::string_view struct_size_macro,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_accessor_start",
        {
            {"struct_name", std::string{c_struct_name}},
            {"struct_size_macro", std::string{struct_size_macro}}
        }
    );
}

inline std::string format_vtable_accessor_end([[maybe_unused]] const std::filesystem::path& repo_root)
{
    return "\n                };\n\n                return &vtable;\n            }\n";
}

inline std::string format_vtable_field_destroy(
    std::string_view slot_name,
    std::string_view cc_class_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_destroy",
        {
            {"slot_name", std::string{slot_name}},
            {"class_name", std::string{cc_class_name}}
        }
    );
}

inline std::string format_vtable_field_get_name(
    std::string_view slot_name,
    std::string_view cc_class_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_string_accessor",
        {
            {"slot_name", std::string{slot_name}},
            {"class_name", std::string{cc_class_name}},
            {"param_type", "TF_String*"},
            {"param_name", "out"}
        }
    );
}

inline std::string format_vtable_field_generic_start(std::string_view slot_name)
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_generic_start",
        {{"slot_name", std::string{slot_name}}}
    );
}

inline std::string format_vtable_field_generic_middle(
    std::string_view cc_class_name,
    std::string_view slot_name,
    std::string_view self_param_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_generic_middle",
        {
            {"class_name", std::string{cc_class_name}},
            {"slot_name", std::string{slot_name}},
            {"self_param_name", std::string{self_param_name}},
            {"trailing_return", ""}
        }
    );
}

inline std::string format_vtable_field_generic_end(
    std::string_view status_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "vtable_field_generic_end",
        {
            {"status_name", std::string{status_name}},
            {"error_return", ""},
            {"success_return", ""}
        }
    );
}

inline std::string format_method_body_start(
    std::string_view method_name,
    std::string_view namespace_name,
    [[maybe_unused]] const std::filesystem::path& repo_root
)
{
    return cc::templating::TemplateRenderer::render_template(
        "method_body_start",
        {
            {"namespace_name", std::string{namespace_name}},
            {"method_name", std::string{method_name}},
            {"result_prefix", ""}
        }
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

} // namespace cc_abi_gen::helper
