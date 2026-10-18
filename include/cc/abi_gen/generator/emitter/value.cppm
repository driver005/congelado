module;

#include <cctype>
#include <nlohmann/json.hpp>

export module cc_abi_gen_generator:value_emitter;

import std;
import cc_templating;
import cc_abi_gen_parser;

export namespace cc_abi_gen::generator::emitter {

// Renders the wrapper class of a plain C struct or enum.
class ValueEmitter
{
public:
    ValueEmitter(std::string_view folder_name, std::string_view namespace_name) :
        m_folder_name{folder_name},
        m_namespace_name{namespace_name}
    {
    }

    std::expected<std::string, std::string>
    render(const parser::value::Model& model, std::string_view module_name) const
    {
        nlohmann::json data;
        data["folder_name"] = m_folder_name;
        data["namespace_name"] = m_namespace_name;
        data["header_path"] = model.get_header_path();
        data["module_name"] = module_name;
        data["partition"] = model.to_file_name();
        data["class_name"] = model.get_class_name();
        data["c_name"] = model.get_c_name();

        if (model.get_kind() == parser::value::Kind::Enum) {
            data["enumerators"] = enumerators(model);

            return cc::templating::TemplateRenderer::render_template_json("value_enum", data);
        }

        data["has_struct_size"] = model.has_struct_size();
        data["fields"] = fields(model);
        data["all_scalar"] = all_scalar(model);

        return cc::templating::TemplateRenderer::render_template_json("value_struct", data);
    }

private:
    // True when every field is a plain value, so the struct can be built from its fields in
    // declaration order.
    static bool all_scalar(const parser::value::Model& model)
    {
        const auto fields = model.get_fields();

        return !fields.empty() && std::ranges::none_of(
                                      fields,
                                      [](const parser::value::Field& field)
                                      {
                                          return field.is_array() || field.is_record();
                                      }
                                  );
    }

    // field_name -> FieldName
    static std::string camel(std::string_view name)
    {
        std::string result;
        bool upper = true;
        for (const char character: name) {
            if (character == '_') {
                upper = true;
                continue;
            }
            result += upper ? static_cast<char>(std::toupper(static_cast<unsigned char>(character)))
                            : character;
            upper = false;
        }

        return result;
    }

    static nlohmann::json fields(const parser::value::Model& model)
    {
        nlohmann::json list = nlohmann::json::array();
        for (const parser::value::Field& field: model.get_fields()) {
            list.push_back(
                {{"name", field.get_name()},
                 {"camel", camel(field.get_name())},
                 {"type", field.get_type()},
                 {"extent", field.get_extent()},
                 {"array", field.is_array()},
                 {"record", field.is_record()}}
            );
        }

        return list;
    }

    // Strips the prefix that every enumerator shares, up to its last underscore.
    // TF_MEMORY_SPACE_DEVICE, TF_MEMORY_SPACE_UNIFIED -> k_device, k_unified
    static nlohmann::json enumerators(const parser::value::Model& model)
    {
        const auto names = model.get_enumerators();
        std::size_t common = 0;
        if (!names.empty()) {
            common = names.front().size();
            for (const std::string& name: names) {
                std::size_t index = 0;
                while (index < common && index < name.size() &&
                       name[index] == names.front()[index]) {
                    ++index;
                }
                common = index;
            }
            const auto underscore = names.front().rfind('_', common == 0 ? 0 : common - 1);
            common = underscore == std::string::npos || common == 0 ? 0 : underscore + 1;
        }

        nlohmann::json list = nlohmann::json::array();
        for (const std::string& name: names) {
            std::string rest = name.substr(std::min(common, name.size() - 1));
            for (char& character: rest) {
                character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            }
            list.push_back({{"name", "k_" + rest}, {"c_name", name}});
        }

        return list;
    }

    std::string m_folder_name;
    std::string m_namespace_name;
};

} // namespace cc_abi_gen::generator::emitter
