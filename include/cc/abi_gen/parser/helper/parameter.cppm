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

    // True for a pointer to a struct/record type (e.g. TF_Status, TF_Buffer); false for raw scalar pointers (e.g. int64_t*) and by-value params, which have no pointee at all. Handle types no longer carry a distinguishing suffix — whether a given pointee actually names a registered domain (as opposed to a plain value struct like TF_Job_Options) is decided by looking it up in the registry, not by this check alone.
    bool has_pointee() const noexcept
    {
        return !m_pointee_name.empty();
    }

    // Registry lookup key for this parameter's pointee. The registry is keyed by the Ops struct's own tag (e.g. "TF_StatusOps"), but a parameter referencing that domain names its handle instead (e.g. "TF_Status* status") — appending "Ops" bridges the two.
    std::string get_registry_key() const noexcept
    {
        return m_pointee_name + "Ops";
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
