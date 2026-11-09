export module cc_abi_gen_classify:classes;

import std;
import :function;
import :parms;

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

        // Class Doxygen Header
        if (!m_doxygen_comment.empty()) {
            code += "/**\n";
            code += std::format(" * @brief {}\n", m_doxygen_comment);
            code += " */\n";
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
        if (!m_functions.empty()) {
            code += "public:\n";
            for (const Function& func: m_functions) {
                code += func.generate_code("    ");
            }
        }

        // Member variables block (Private)
        if (!m_member_variables.empty()) {
            if (!m_functions.empty()) {
                code += '\n';
            }
            code += "private:\n";
            for (const std::string& var: m_member_variables) {
                code += std::format("    {}\n", var);
            }
        }

        code += "};\n";

        return code;
    }

    Classify& set_class_doxygen(std::string comment)
    {
        m_doxygen_comment = std::move(comment);
        return *this;
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

    Classify& set_member_variable(std::string type, std::string name, std::string default_val = "")
    {
        if (default_val.empty()) {
            m_member_variables.push_back(std::format("{} m_{};", type, name));
        } else {
            m_member_variables.push_back(std::format("{} m_{}{{{}}};", type, name, default_val));
        }
        return *this;
    }

private:
    std::string m_class_name;
    std::vector<std::string> m_base_classes;
    std::vector<Function> m_functions;
    std::vector<std::string> m_member_variables;
    std::string m_doxygen_comment;
};

} // namespace cc_abi_gen::classify
