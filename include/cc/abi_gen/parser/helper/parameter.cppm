export module cc_abi_gen_parser:helper_parameter;

import std;

export namespace cc_abi_gen::parser::helper {

class Parameter
{
public:
    Parameter(std::string type, std::string pointee_name, std::string name) :
        m_type(type),
        m_pointee_name(pointee_name),
        m_name(name)
    {
    }

    // Example: TF_TString
    std::string_view get_type() const
    {
        return m_type;
    }

    // Example: TF_Status
    std::string_view get_pointee_name() const
    {
        return m_pointee_name;
    }

    // Example: argument_name
    std::string_view get_name() const
    {
        return m_name;
    }

private:
    std::string m_type;
    std::string m_pointee_name;
    std::string m_name;
};

} // namespace cc_abi_gen::parser::helper
