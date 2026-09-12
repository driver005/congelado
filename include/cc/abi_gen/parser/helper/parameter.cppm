export module cc_abi_gen_parser:helper_parameter;

import std;

export namespace cc_abi_gen::parser::helper {

class Parameter
{
public:
    Parameter() = default;

    Parameter(std::string type, std::string pointee_name, std::string name) :
        m_type(type),
        m_pointee_name(pointee_name),
        m_name(name)
    {
    }

    ~Parameter() = default;
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) = default;
    Parameter& operator=(Parameter&&) = default;

    Parameter& add_type(std::string&& type) noexcept
    {
        m_type = std::move(type);
        return *this;
    }

    Parameter& add_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
        return *this;
    }

    Parameter& add_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
        return *this;
    }

    void set_type(std::string&& type) noexcept
    {
        m_type = std::move(type);
    }

    void set_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
    }

    void set_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
    }

    // Example: TF_TString
    std::string& get_type() noexcept
    {
        return m_type;
    }

    // Example: TF_Status
    std::string& get_pointee_name() noexcept
    {
        return m_pointee_name;
    }

    // Example: argument_name
    std::string& get_name() noexcept
    {
        return m_name;
    }

    // Example: TF_TString
    std::string_view get_type() const noexcept
    {
        return m_type;
    }

    // Example: TF_Status
    std::string_view get_pointee_name() const noexcept
    {
        return m_pointee_name;
    }

    // Example: argument_name
    std::string_view get_name() const noexcept
    {
        return m_name;
    }

private:
    std::string m_type;
    std::string m_pointee_name;
    std::string m_name;
};

} // namespace cc_abi_gen::parser::helper
