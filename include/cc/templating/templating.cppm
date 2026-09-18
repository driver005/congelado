module;

#include <expected>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include "include/cc/templating/generated/cc_templating_templates.h"

export module cc_templating;

import std;

export namespace cc::templating {

class TemplateRenderer
{
public:
    static std::expected<std::string, std::string> render(
        std::string_view pattern,
        const std::vector<std::pair<std::string, std::string>>& variables
    ) noexcept
    {
        nlohmann::json data = nlohmann::json::object();

        for (const auto& variable: variables) {
            data[variable.first] = variable.second;
        }

        return render(pattern, data);
    }

    static std::expected<std::string, std::string> render(
        std::string_view pattern,
        const nlohmann::json& data
    ) noexcept
    {
        inja::Environment environment;

        try {
            return environment.render(pattern, data);
        } catch (const inja::RenderError& e) {
            return std::unexpected(e.what());
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static std::expected<std::string, std::string> render_template(
        std::string_view template_name,
        const std::vector<std::pair<std::string, std::string>>& variables
    ) noexcept
    {
        nlohmann::json data = nlohmann::json::object();

        for (const auto& variable: variables) {
            data[variable.first] = variable.second;
        }

        return render(find_template(template_name), data);
    }

    static std::expected<std::string, std::string> render_template_json(
        std::string_view template_name,
        const nlohmann::json& data
    ) noexcept
    {
        return render(find_template(template_name), data);
    }

private:
    static std::string_view find_template(std::string_view template_name)
    {
        static const std::unordered_map<std::string_view, std::string_view> templates{
            {"class_body_builder", cc_templating_generated::k_class_body_builder},
            {"inheritance_sonic", cc_templating_generated::k_inheritance_sonic},
            {"class_body_sonic", cc_templating_generated::k_class_body_sonic},
            {"module_header", cc_templating_generated::k_module_header},
            {"module_footer", cc_templating_generated::k_module_footer},
            {"module_base", cc_templating_generated::k_module_base},
            {"parameter", cc_templating_generated::k_parameter},
            {"method_signature", cc_templating_generated::k_method_signature},
            {"vtable_accessor_start", cc_templating_generated::k_vtable_accessor_start},
            {"vtable_field_destroy", cc_templating_generated::k_vtable_field_destroy},
            {"vtable_field_generic_start", cc_templating_generated::k_vtable_field_generic_start},
            {"vtable_field_generic_middle", cc_templating_generated::k_vtable_field_generic_middle},
            {"vtable_field_generic_end", cc_templating_generated::k_vtable_field_generic_end},
            {"method_body_start", cc_templating_generated::k_method_body_start},
            {"method_decl_string_accessor", cc_templating_generated::k_method_decl_string_accessor},
            {"vtable_field_string_accessor",
             cc_templating_generated::k_vtable_field_string_accessor},
            {"method_string_accessor_sonic",
             cc_templating_generated::k_method_string_accessor_sonic},
            {"method_decl_codec_pair", cc_templating_generated::k_method_decl_codec_pair},
            {"vtable_field_codec_pair", cc_templating_generated::k_vtable_field_codec_pair},
            {"method_codec_pair_sonic", cc_templating_generated::k_method_codec_pair_sonic},
            {"method_decl_typed_return", cc_templating_generated::k_method_decl_typed_return},
            {"vtable_field_typed_return", cc_templating_generated::k_vtable_field_typed_return},
            {"method_typed_return_sonic", cc_templating_generated::k_method_typed_return_sonic}
        };

        return templates.at(template_name);
    }
};

} // namespace cc::templating
