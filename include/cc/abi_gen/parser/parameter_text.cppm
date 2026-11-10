export module cc_abi_gen_parser:parameter_text;

import std;

export namespace cc_abi_gen::parser {

// What one C parameter turns into on one side of the boundary. Empty means absent.
class ParameterText
{
public:
    ParameterText& add_before(const std::string& code)
    {
        m_before += code;
        return *this;
    }

    ParameterText& add_after(const std::string& code)
    {
        m_after += code;
        return *this;
    }

    ParameterText& set_type(std::string type)
    {
        m_type = std::move(type);
        return *this;
    }

    ParameterText& set_argument(std::string argument)
    {
        m_argument = std::move(argument);
        return *this;
    }

    ParameterText& set_before(std::string code)
    {
        m_before = std::move(code);
        return *this;
    }

    ParameterText& set_after(std::string code)
    {
        m_after = std::move(code);
        return *this;
    }

    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const std::string& get_argument() const noexcept
    {
        return m_argument;
    }

    [[nodiscard]] const std::string& get_before() const noexcept
    {
        return m_before;
    }

    [[nodiscard]] const std::string& get_after() const noexcept
    {
        return m_after;
    }

private:
    std::string m_type;
    std::string m_argument;
    std::string m_before;
    std::string m_after;
};

} // namespace cc_abi_gen::parser
