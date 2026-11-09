export module cc_abi_gen_classify:module_builder;

import std;
import :parms;
import :function;
import :classes;

export namespace cc_abi_gen::classify {

class ModuleBuilder
{
public:
    ModuleBuilder() = default;

    explicit ModuleBuilder(std::string module_name) :
        m_module_name(std::move(module_name))
    {
    }

    ~ModuleBuilder() = default;

    ModuleBuilder(const ModuleBuilder& other) = default;
    ModuleBuilder& operator=(const ModuleBuilder& other) = default;
    ModuleBuilder(ModuleBuilder&& other) noexcept = default;
    ModuleBuilder& operator=(ModuleBuilder&& other) noexcept = default;

    [[nodiscard]] ModuleBuilder& set_import(std::string import_name)
    {
        m_imports.push_back(std::move(import_name));
        return *this;
    }

    [[nodiscard]] ModuleBuilder& set_namespace(std::string space)
    {
        m_namespaces.push_back(std::move(space));
        return *this;
    }

    [[nodiscard]] ModuleBuilder& set_class(Classify cls)
    {
        m_classes.push_back(std::move(cls));
        return *this;
    }

    [[nodiscard]] ModuleBuilder& set_global_fragment(std::string fragment)
    {
        m_global_fragment = std::move(fragment);
        return *this;
    }

    [[nodiscard]] ModuleBuilder& set_is_export(bool is_export) noexcept
    {
        m_is_export_module = is_export;
        return *this;
    }

    [[nodiscard]] std::string generate_code() const
    {
        std::string code;

        // Banner + `module;` + #includes, emitted before the module declaration.
        code += m_global_fragment;

        // Module declaration
        if (m_is_export_module) {
            code += std::format("export module {};\n\n", m_module_name);
        } else {
            code += std::format("module {};\n\n", m_module_name);
        }

        // Imports
        if (!m_imports.empty()) {
            code += m_imports |
                    std::views::transform(
                        [](const std::string& imp)
                        {
                            return std::format("import {};\n", imp);
                        }
                    ) |
                    std::views::join_with(std::string_view("")) | std::ranges::to<std::string>();
            code += '\n';
        }

        // Namespace
        std::string namespace_path;
        if (!m_namespaces.empty()) {
            namespace_path = m_namespaces | std::views::join_with(std::string_view("::")) |
                             std::ranges::to<std::string>();
            code += std::format("export namespace {} {{\n\n", namespace_path);
        }

        // Classes
        if (!m_classes.empty()) {
            std::string classes_code = m_classes |
                                       std::views::transform(
                                           [](const Classify& cls)
                                           {
                                               return cls.generate_header() + "\n";
                                           }
                                       ) |
                                       std::views::join_with(std::string_view("\n")) |
                                       std::ranges::to<std::string>();
            code += classes_code;
        }

        // Namespace
        if (!m_namespaces.empty()) {
            code += std::format("}} // namespace {}\n", namespace_path);
        }

        return code;
    }

private:
    std::string m_module_name;
    std::string m_global_fragment;
    bool m_is_export_module{true};
    std::vector<std::string> m_imports;
    std::vector<std::string> m_namespaces;
    std::vector<Classify> m_classes;
};

} // namespace cc_abi_gen::classify
