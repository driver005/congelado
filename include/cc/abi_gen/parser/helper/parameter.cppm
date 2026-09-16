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
    std::string_view get_type() const noexcept
    {
        return m_type;
    }

    // Example: TF_Status_Handle
    std::string_view get_pointee_name() const noexcept
    {
        return m_pointee_name;
    }

    // True for a pointer to an opaque "_Handle" type; false for raw scalar pointers (e.g. int64_t*) and by-value params.
    bool is_handle() const noexcept
    {
        return !m_pointee_name.empty() && m_pointee_name.ends_with("_Handle");
    }

    // Owning Ops-struct name, with "_Handle" suffix stripped if present.
    std::string get_registry_key() const noexcept
    {
        static constexpr std::string_view suffix = "_Handle";
        if (m_pointee_name.size() > suffix.size() && m_pointee_name.ends_with(suffix)) {
            return m_pointee_name.substr(0, m_pointee_name.size() - suffix.size());
        }
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
