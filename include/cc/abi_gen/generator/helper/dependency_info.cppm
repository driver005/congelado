export module cc_abi_gen_generator:helper_dependency_info;

import std;

export namespace cc_abi_gen::helper {

class DependencyInfo
{
public:
    DependencyInfo(
        std::string&& sonic_type,
        std::string&& struct_name,
        std::string&& member_name,
        std::string&& handle_name,
        std::string&& header_path
    ) :
        m_sonic_type{std::move(sonic_type)},
        m_struct_name{std::move(struct_name)},
        m_member_name{std::move(member_name)},
        m_handle_name{std::move(handle_name)},
        m_header_path{std::move(header_path)}
    {
    }

    const std::string& get_sonic_type() const noexcept
    {
        return m_sonic_type;
    }

    const std::string& get_struct_name() const noexcept
    {
        return m_struct_name;
    }

    const std::string& get_member_name() const noexcept
    {
        return m_member_name;
    }

    const std::string& get_handle_name() const noexcept
    {
        return m_handle_name;
    }

    const std::string& get_header_path() const noexcept
    {
        return m_header_path;
    }

private:
    std::string m_sonic_type;
    std::string m_struct_name;
    std::string m_member_name;
    std::string m_handle_name;
    std::string m_header_path;
};

} // namespace cc_abi_gen::helper
