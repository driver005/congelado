export module cc_abi_gen_parser:helper_paths;

import std;

export namespace cc_abi_gen::parser::helper {

class DomainPaths
{
public:
    DomainPaths() = default;

    // is_extern_domain only picks the output folder (extern/ vs intern/) for on-disk browsing — it
    // never reaches the generated namespace, which stays ice::builder/ice::sonic either way.
    DomainPaths(
        const std::string&& domain,
        const std::filesystem::path&& repo_root,
        const std::filesystem::path&& output_root,
        bool is_extern_domain
    ) :
        m_header{repo_root / "include/c/extern" / domain / (domain + ".h")}
    {
        const std::filesystem::path domain_root =
            output_root / (is_extern_domain ? "extern" : "intern") / domain;

        if (count_headers(repo_root, domain) > 1) {
            m_builder_cppm = domain_root / "builder" / (domain + ".cppm");
            m_sonic_cppm = domain_root / "sonic" / (domain + ".cppm");
        } else {
            m_builder_cppm = domain_root / "builder.cppm";
            m_sonic_cppm = domain_root / "sonic.cppm";
        }
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
    const std::filesystem::path& get_header() const noexcept
    {
        return m_header;
    }

    // Example: include/cc/abi/intern/string/builder.cppm (or include/cc/abi/extern/io/builder/io.cppm)
    const std::filesystem::path& get_builder_cppm() const noexcept
    {
        return m_builder_cppm;
    }

    // Example: include/cc/abi/intern/string/sonic.cppm (or include/cc/abi/extern/io/sonic/io.cppm)
    const std::filesystem::path& get_sonic_cppm() const noexcept
    {
        return m_sonic_cppm;
    }


private:
    // Counts *.h files directly under include/c/extern/<domain> — more than one means the
    // domain's output gets its own builder/sonic subfolders instead of flat files.
    std::size_t count_headers(const std::filesystem::path& repo_root, const std::string& domain)
        const
    {
        std::size_t count = 0;

        std::error_code error;
        std::filesystem::path domain_extern_root = repo_root / "include/c/extern" / domain;

        for (const auto& entry:
             std::filesystem::directory_iterator{domain_extern_root, error}) {
            if (entry.is_regular_file() && entry.path().extension() == ".h") {
                ++count;
            }
        }

        return count;
    }

    std::filesystem::path m_header;
    std::filesystem::path m_builder_cppm;
    std::filesystem::path m_sonic_cppm;
};

} // namespace cc_abi_gen::parser::helper
