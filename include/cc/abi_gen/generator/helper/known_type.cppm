export module cc_abi_gen_generator:helper_known_type;

import std;

export namespace cc_abi_gen::helper {

class KnownType
{
public:
    KnownType() = default;

    KnownType(
        std::string&& pointee_name,
        std::string&& cpp_parameter_type,
        std::string&& wrap_format,
        std::string&& unwrap_format
    ) :
        m_pointee_name{std::move(pointee_name)},
        m_cpp_parameter_type{std::move(cpp_parameter_type)},
        m_wrap_format{std::move(wrap_format)},
        m_unwrap_format{std::move(unwrap_format)}
    {
    }

    ~KnownType() = default;
    KnownType(const KnownType&) = delete;
    KnownType& operator=(const KnownType&) = delete;
    KnownType(KnownType&&) = default;
    KnownType& operator=(KnownType&&) = default;

    KnownType& add_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
        return *this;
    }

    KnownType& add_cpp_parameter_type(std::string&& cpp_parameter_type) noexcept
    {
        m_cpp_parameter_type = std::move(cpp_parameter_type);
        return *this;
    }

    KnownType& add_wrap_format(std::string&& wrap_format) noexcept
    {
        m_wrap_format = std::move(wrap_format);
        return *this;
    }

    KnownType& add_unwrap_format(std::string&& unwrap_format) noexcept
    {
        m_unwrap_format = std::move(unwrap_format);
        return *this;
    }

    std::string wrape_cc_type(const std::string& argument_name)
    {
        return std::vformat(known.m_wrap_format, std::make_format_args(argument_name));
    }

    std::string unwrape_cc_type(const std::string& argument_name)
    {
        return std::vformat(known.m_unwrap_format, std::make_format_args(argument_name));
    }

    void set_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
    }

    void set_cpp_parameter_type(std::string&& cpp_parameter_type) noexcept
    {
        m_cpp_parameter_type = std::move(cpp_parameter_type);
    }

    void set_wrap_format(std::string&& wrap_format) noexcept
    {
        m_wrap_format = std::move(wrap_format);
    }

    void set_unwrap_format(std::string&& unwrap_format) noexcept
    {
        m_unwrap_format = std::move(unwrap_format);
    }

    // Example: TF_TString
    const std::string& get_pointee_name() const

    {
        return m_pointee_name;
    }

    // Example: const ice::String &
    const std::string& get_cpp_parameter_type() const
    {
        return m_cpp_parameter_type;
    }

    // Example: ice::String::create({})
    const std::string& get_wrap_format() const
    {
        return m_wrap_format;
    }

    // Example: "{}.get_handle()",
    const std::string& get_unwrap_format() const
    {
        return m_unwrap_format;
    }

private:
    std::string m_pointee_name;
    std::string m_cpp_parameter_type;
    std::string m_wrap_format;
    std::string m_unwrap_format;
};

} // namespace cc_abi_gen::helper
