export module cc_abi_gen_parser:helper_paths;

import std;

export namespace cc_abi_gen::parser::helper {

class DomainPaths
{
public:
    DomainPaths(
        const std::string&& domain,
        const std::filesystem::path&& repo_root,
        const std::filesystem::path&& output_root
    ) :
        m_header{repo_root / "include/c/extern" / domain / (domain + ".h")},
        m_builder_cppm{output_root / "builder" / domain / (domain + ".cppm")},
        m_sonic_cppm{output_root / "sonic" / domain / (domain + ".cppm")}
    {
    }

    // Example: include/c/extern/string/string.h
    const std::filesystem::path& get_header() const
    {
        return m_header;
    }

    // Example: include/cc/abi/builder/string/string.cppm
    const std::filesystem::path& get_builder_cppm() const
    {
        return m_builder_cppm;
    }

    // Example: include/cc/abi/sonic/string/string.cppm
    const std::filesystem::path& get_sonic_cppm() const
    {
        return m_sonic_cppm;
    }


private:
    std::filesystem::path m_header;
    std::filesystem::path m_builder_cppm;
    std::filesystem::path m_sonic_cppm;
};

} // namespace cc_abi_gen::parser::helper
