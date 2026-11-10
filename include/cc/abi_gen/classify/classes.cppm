export module cc_abi_gen_classify:classes;

import std;
import :function;
import :parms;
import :utils;

export namespace cc_abi_gen::classify {

class Classify
{
public:
    Classify() = default;

    explicit Classify(std::string class_name) :
        m_class_name(std::move(class_name))
    {
    }

    ~Classify() = default;

    Classify(const Classify& other) = default;
    Classify& operator=(const Classify& other) = default;
    Classify(Classify&& other) noexcept = default;
    Classify& operator=(Classify&& other) noexcept = default;

    [[nodiscard]] std::string generate_header() const
    {
        std::string code;

        if (!m_template_params.empty()) {
            code += std::format("template<{}>\n", m_template_params);
        }

        // Header class signature line
        code += std::format("class {} ", m_class_name);
        if (!m_base_classes.empty()) {
            code += ": ";
            code += m_base_classes | std::views::join_with(std::string_view(", ")) |
                    std::ranges::to<std::string>();
        }
        code += "{\n";

        // Functions block (Public)
        if (!m_functions.empty() || !m_public_raw.empty()) {
            code += "public:\n";
            if (!m_public_raw.empty()) {
                code += m_public_raw;
                if (!m_functions.empty()) {
                    code += '\n';
                }
            }
            for (const Function& func: m_functions) {
                code += func.generate_code("    ");
            }
        }

        // Functions and member variables block (Protected)
        const bool HAS_PUBLIC = !m_functions.empty() || !m_public_raw.empty();
        const bool HAS_PROTECTED =
            !m_protected_functions.empty() || !m_protected_member_variables.empty();
        code += generate_protected(HAS_PUBLIC);
        code += generate_private(HAS_PUBLIC || HAS_PROTECTED);

        code += "};\n";

        return code;
    }

    Classify& set_base_class(std::string baseClass, std::string access = "public")
    {
        m_base_classes.push_back(std::format("{} {}", access, baseClass));
        return *this;
    }

    Classify& set_function(Function func)
    {
        m_functions.push_back(std::move(func));
        return *this;
    }

    Classify& set_member_variable(
        const std::string& type,
        const std::string& name,
        const std::optional<std::string>& default_value = std::nullopt
    )
    {
        m_member_variables.push_back(member_declaration(type, name, default_value));
        return *this;
    }

    // Function in the protected block (e.g. a base class constructor).
    Classify& set_protected_function(Function func)
    {
        m_protected_functions.push_back(std::move(func));
        return *this;
    }

    // Member variable in the protected block; `mutable` goes in the type.
    Classify& set_protected_member_variable(
        const std::string& type,
        const std::string& name,
        const std::optional<std::string>& default_value = std::nullopt
    )
    {
        m_protected_member_variables.push_back(member_declaration(type, name, default_value));
        return *this;
    }

    // Class template head without brackets, e.g. `typename OpsStruct, typename HandleStruct`.
    Classify& set_template(std::string template_params)
    {
        m_template_params = std::move(template_params);
        return *this;
    }

    // C struct this class was parsed from. Example: TF_StringOps
    Classify& set_c_name(std::string c_name)
    {
        m_c_name = std::move(c_name);
        return *this;
    }

    Classify& set_kind(Kind kind) noexcept
    {
        m_kind = kind;
        return *this;
    }

    // Repo-relative C header this class was parsed from. Example: include/c/intern/string.h
    Classify& set_header_path(std::string header_path)
    {
        m_header_path = std::move(header_path);
        return *this;
    }

    // Verbatim text inside the public block (e.g. a nested enum declaration).
    Classify& set_public_raw(std::string raw)
    {
        m_public_raw = std::move(raw);
        return *this;
    }

    // One blank line inside the member block (group separator).
    Classify& set_member_gap()
    {
        m_member_variables.emplace_back();
        return *this;
    }

    [[nodiscard]] const std::string& get_class_name() const noexcept
    {
        return m_class_name;
    }

    [[nodiscard]] std::span<const Function> get_functions() const noexcept
    {
        return m_functions;
    }

    [[nodiscard]] const std::string& get_c_name() const noexcept
    {
        return m_c_name;
    }

    [[nodiscard]] const std::string& get_header_path() const noexcept
    {
        return m_header_path;
    }

    [[nodiscard]] Kind get_kind() const noexcept
    {
        return m_kind;
    }

private:
    [[nodiscard]] std::string generate_protected(bool after_public) const
    {
        if (m_protected_functions.empty() && m_protected_member_variables.empty()) {
            return {};
        }

        std::string code = after_public ? "\nprotected:\n" : "protected:\n";
        for (const Function& func: m_protected_functions) {
            code += func.generate_code("    ");
        }
        if (!m_protected_functions.empty() && !m_protected_member_variables.empty()) {
            code += '\n';
        }
        for (const std::string& var: m_protected_member_variables) {
            code += std::format("    {}\n", var);
        }

        return code;
    }

    [[nodiscard]] std::string generate_private(bool after_other_block) const
    {
        if (m_member_variables.empty()) {
            return {};
        }

        std::string code = after_other_block ? "\nprivate:\n" : "private:\n";
        for (const std::string& var: m_member_variables) {
            // An empty entry is a set_member_gap() marker: one blank line between groups.
            code += var.empty() ? "\n" : std::format("    {}\n", var);
        }

        return code;
    }

    static std::string member_declaration(
        const std::string& type,
        const std::string& name,
        const std::optional<std::string>& default_value
    )
    {
        if (!default_value.has_value()) {
            return std::format("{} m_{};", type, name);
        }
        if (default_value->empty()) {
            return std::format("{} m_{}{{}};", type, name);
        }

        return std::format("{} m_{}{{{}}};", type, name, *default_value);
    }

    std::string m_class_name;
    std::vector<std::string> m_base_classes;
    std::vector<Function> m_functions;
    std::vector<std::string> m_member_variables;
    std::vector<Function> m_protected_functions;
    std::vector<std::string> m_protected_member_variables;
    std::string m_public_raw;
    std::string m_template_params;
    std::string m_c_name;
    std::string m_header_path;
    Kind m_kind{Kind::VTABLE};
};

} // namespace cc_abi_gen::classify
