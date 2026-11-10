export module cc_abi_gen_classify:function;

import std;
import :parms;
import :utils;

export namespace cc_abi_gen::classify {

class Function
{
public:
    Function() = default;

    explicit Function(
        std::string name,
        std::string return_type = "void",
        FunctionType type = FunctionType::STANDARD
    ) :
        m_name(std::move(name)),
        m_return_type(std::move(return_type)),
        m_type(type)
    {
    }

    ~Function() = default;

    Function(const Function& other) = default;
    Function& operator=(const Function& other) = default;
    Function(Function&& other) noexcept = default;
    Function& operator=(Function&& other) noexcept = default;

    [[nodiscard]] std::string generate_signature(bool include_defaults = true) const
    {
        std::string sig;

        if (m_is_constexpr) {
            sig += "constexpr ";
        }
        if (m_is_static) {
            sig += "static ";
        }
        if (m_is_explicit) {
            sig += "explicit ";
        }
        if (m_is_virtual) {
            sig += "virtual ";
        }

        if (m_type == FunctionType::STANDARD) {
            sig += std::format("{} ", m_return_type);
        }

        sig += m_name + "(";

        std::string params_str = m_parameters |
                                 std::views::transform(
                                     [include_defaults](const Param& param)
                                     {
                                         return param.to_string(include_defaults);
                                     }
                                 ) |
                                 std::views::join_with(std::string_view(", ")) |
                                 std::ranges::to<std::string>();

        sig += params_str;
        sig += ')';

        if (m_is_const) {
            sig += " const";
        }
        if (m_is_noexcept) {
            sig += " noexcept";
        }

        return sig;
    }

    [[nodiscard]] std::string generate_code(std::string_view indent = "    ") const
    {
        std::string code;

        if (!m_template_params.empty()) {
            code += std::format("{}template<{}>\n", indent, m_template_params);
        }

        code += std::format("{}{}", indent, generate_signature(true));

        // Declaration-only forms: `= 0`, `= delete`, `= default`.
        if (m_is_pure_virtual) {
            return code + " = 0;\n";
        }
        if (m_is_deleted) {
            return code + " = delete;\n";
        }
        if (m_is_defaulted) {
            return code + " = default;\n";
        }

        if (!m_initializer_list.empty()) {
            code += std::format(" :\n{}{}", indent, m_initializer_list);
        }
        code += '\n';

        code += std::format("{}{{\n", indent);

        for (const std::string& line: m_body_lines) {
            code += std::format("{}    {}\n", indent, line);
        }

        code += std::format("{}}}\n", indent);

        return code;
    }

    Function& set_param(Param param)
    {
        m_parameters.push_back(std::move(param));
        return *this;
    }

    Function& set_param(
        std::string type,
        std::string name,
        std::optional<std::string> default_value = std::nullopt
    )
    {
        m_parameters.emplace_back(std::move(type), std::move(name), std::move(default_value));
        return *this;
    }

    Function& set_body_from_raw_string(std::string_view raw_code)
    {
        auto lines = raw_code | std::views::split('\n') |
                     std::views::transform(
                         [](auto&& rng)
                         {
                             return std::string(rng.begin(), rng.end());
                         }
                     );

        for (const auto& line: lines) {
            m_body_lines.push_back(line);
        }

        // Drop the empty element the trailing newline produces.
        if (!m_body_lines.empty() && m_body_lines.back().empty()) {
            m_body_lines.pop_back();
        }

        return *this;
    }

    Function& set_body_line(std::string line)
    {
        m_body_lines.push_back(std::move(line));
        return *this;
    }

    Function& set_explicit(bool value = true) noexcept
    {
        m_is_explicit = value;
        return *this;
    }

    Function& set_pure_virtual(bool value = true) noexcept
    {
        m_is_pure_virtual = value;
        return *this;
    }

    Function& set_deleted(bool value = true) noexcept
    {
        m_is_deleted = value;
        return *this;
    }

    Function& set_defaulted(bool value = true) noexcept
    {
        m_is_defaulted = value;
        return *this;
    }

    // Constructor initializer list, e.g. `Runtime(ops)` — emitted after `noexcept :`.
    Function& set_initializer(std::string initializer)
    {
        m_initializer_list = std::move(initializer);
        return *this;
    }

    // Template head without brackets, e.g. `typename HandleT`.
    Function& set_template(std::string template_params)
    {
        m_template_params = std::move(template_params);
        return *this;
    }

    Function& set_const(bool value = true) noexcept
    {
        m_is_const = value;
        return *this;
    }

    Function& set_noexcept(bool value = true) noexcept
    {
        m_is_noexcept = value;
        return *this;
    }

    Function& set_virtual(bool value = true) noexcept
    {
        m_is_virtual = value;
        return *this;
    }

    Function& set_static(bool value = true) noexcept
    {
        m_is_static = value;
        return *this;
    }

    Function& set_constexpr(bool value = true) noexcept
    {
        m_is_constexpr = value;
        return *this;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_return_type() const noexcept
    {
        return m_return_type;
    }

    [[nodiscard]] std::span<const Param> get_parameters() const noexcept
    {
        return m_parameters;
    }

    [[nodiscard]] FunctionType get_type() const noexcept
    {
        return m_type;
    }

private:
    std::string m_name;
    std::string m_return_type{"void"};
    FunctionType m_type{FunctionType::STANDARD};
    std::vector<Param> m_parameters;
    std::vector<std::string> m_body_lines;
    std::string m_initializer_list;
    std::string m_template_params;

    bool m_is_const{false};
    bool m_is_noexcept{false};
    bool m_is_virtual{false};
    bool m_is_static{false};
    bool m_is_constexpr{false};
    bool m_is_explicit{false};
    bool m_is_pure_virtual{false};
    bool m_is_deleted{false};
    bool m_is_defaulted{false};
};

} // namespace cc_abi_gen::classify
