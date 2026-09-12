export module cc_abi_gen_parser:helper_paths;

import std;

export namespace cc_abi_gen::parser::helper {

class DomainPaths
{
public:
    DomainPaths() = default;

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

    ~DomainPaths() = default;
    DomainPaths(const DomainPaths&) = delete;
    DomainPaths& operator=(const DomainPaths&) = delete;
    DomainPaths(DomainPaths&&) = default;
    DomainPaths& operator=(DomainPaths&&) = default;

    DomainPaths& add_header(const std::filesystem::path&& header) noexcept
    {
        m_header = std::move(header);
        return *this;
    }

    DomainPaths& add_builder_cppm(const std::filesystem::path&& builder_cppm) noexcept
    {
        m_builder_cppm = std::move(builder_cppm);
        return *this;
    }

    DomainPaths& add_sonic_cppm(const std::filesystem::path&& sonic_cppm) noexcept
    {
        m_sonic_cppm = std::move(sonic_cppm);
        return *this;
    }

    void set_header(const std::filesystem::path&& header) noexcept
    {
        m_header = std::move(header);
    }

    void set_builder_cppm(const std::filesystem::path&& builder_cppm) noexcept
    {
        m_builder_cppm = std::move(builder_cppm);
    }

    void set_sonic_cppm(const std::filesystem::path&& sonic_cppm) noexcept
    {
        m_sonic_cppm = std::move(sonic_cppm);
    }

    // Example: include/c/extern/string/string.h
    std::filesystem::path& get_header() noexcept
    {
        return m_header;
    }

    // Example: include/cc/abi/builder/string/string.cppm
    std::filesystem::path& get_builder_cppm() noexcept
    {
        return m_builder_cppm;
    }

    // Example: include/cc/abi/sonic/string/string.cppm
    std::filesystem::path& get_sonic_cppm() noexcept
    {
        return m_sonic_cppm;
    }

    // Example: include/c/extern/string/string.h
    const std::filesystem::path& get_header() const noexcept
    {
        return m_header;
    }

    // Example: include/cc/abi/builder/string/string.cppm
    const std::filesystem::path& get_builder_cppm() const noexcept
    {
        return m_builder_cppm;
    }

    // Example: include/cc/abi/sonic/string/string.cppm
    const std::filesystem::path& get_sonic_cppm() const noexcept
    {
        return m_sonic_cppm;
    }


private:
    std::filesystem::path m_header;
    std::filesystem::path m_builder_cppm;
    std::filesystem::path m_sonic_cppm;
};

} // namespace cc_abi_gen::parser::helper
