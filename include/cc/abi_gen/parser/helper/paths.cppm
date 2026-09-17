export module cc_abi_gen_parser:helper_paths;

import std;

namespace cc_abi_gen::parser::detail {

struct HeaderIndex
{
    std::unordered_map<std::string, std::filesystem::path> exact;
    std::unordered_map<std::string, std::filesystem::path> flat;
    std::unordered_map<std::string, std::filesystem::path> extern_dirs;
};

inline const HeaderIndex& get_header_index(const std::filesystem::path& repo_root)
{
    static HeaderIndex index;
    static bool built = false;

    if (built) {
        return index;
    }

    std::error_code ec;
    auto extern_root = repo_root / "include/c/extern";
    auto intern_root = repo_root / "include/c/intern";

    if (std::filesystem::exists(extern_root)) {
        for (const auto& dir: std::filesystem::directory_iterator{extern_root, ec}) {
            if (!dir.is_directory()) {
                continue;
            }

            auto dir_name = dir.path().filename().string();
            index.extern_dirs[dir_name] = dir.path();

            for (const auto& h: std::filesystem::directory_iterator{dir.path(), ec}) {
                if (!h.is_regular_file() || h.path().extension() != ".h") {
                    continue;
                }
                auto stem = h.path().stem().string();
                index.exact[stem] = h.path();

                std::string flat_stem;
                flat_stem.reserve(stem.size());
                for (char c: stem) {
                    if (c != '_') {
                        flat_stem += c;
                    }
                }
                index.flat[flat_stem] = h.path();
            }
        }
    }

    if (std::filesystem::exists(intern_root)) {
        for (const auto& h: std::filesystem::directory_iterator{intern_root, ec}) {
            if (!h.is_regular_file() || h.path().extension() != ".h") {
                continue;
            }
            auto stem = h.path().stem().string();
            if (stem.starts_with("tf_")) {
                auto domain = stem.substr(3);
                if (!index.exact.contains(domain)) {
                    index.exact[domain] = h.path();
                }

                std::string flat_domain;
                flat_domain.reserve(domain.size());
                for (char c: domain) {
                    if (c != '_') {
                        flat_domain += c;
                    }
                }
                if (!index.flat.contains(flat_domain)) {
                    index.flat[flat_domain] = h.path();
                }
            }
        }
    }

    built = true;
    return index;
}

} // namespace cc_abi_gen::parser::detail

export namespace cc_abi_gen::parser::helper {

class DomainPaths
{
public:
    DomainPaths() = default;

    DomainPaths(
        const std::string&& domain,
        const std::filesystem::path&& repo_root,
        const std::filesystem::path&& output_root,
        bool /*is_extern_domain*/
    ) :
        m_domain{domain}
    {
        const auto& idx = detail::get_header_index(repo_root);

        // 1. Exact stem match
        if (auto it = idx.exact.find(domain); it != idx.exact.end()) {
            m_header = it->second;
        } else {
            // 2. Flatten match (e.g. "pub_sub" → "pubsub")
            std::string flat_domain;
            flat_domain.reserve(domain.size());
            for (char c: domain) {
                if (c != '_') {
                    flat_domain += c;
                }
            }
            if (auto it = idx.flat.find(flat_domain); it != idx.flat.end()) {
                m_header = it->second;
            }
        }

        // 3. Determine m_extern_dir from header path
        auto header_str = m_header.string();
        constexpr auto prefix = "/include/c/extern/";
        auto pos = header_str.find(prefix);
        if (pos != std::string::npos) {
            m_is_extern = true;
            auto rest = header_str.substr(pos + std::string(prefix).size());
            auto slash = rest.find('/');
            if (slash != std::string::npos) {
                m_extern_dir = rest.substr(0, slash);
            }
        }

        // 4. Output directory
        auto out_domain = m_extern_dir.empty() ? domain : m_extern_dir;
        const std::filesystem::path domain_root =
            output_root / (m_is_extern ? "extern" : "intern") / out_domain;

        m_builder_cppm = domain_root / "builder" / (domain + ".cppm");
        m_sonic_cppm = domain_root / "sonic" / (domain + ".cppm");
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

    const std::filesystem::path& get_header() const noexcept
    {
        return m_header;
    }

    const std::filesystem::path& get_builder_cppm() const noexcept
    {
        return m_builder_cppm;
    }

    const std::filesystem::path& get_sonic_cppm() const noexcept
    {
        return m_sonic_cppm;
    }

    bool is_extern() const noexcept
    {
        return m_is_extern;
    }


private:
    std::string m_domain;
    std::string m_extern_dir;
    std::filesystem::path m_header;
    std::filesystem::path m_builder_cppm;
    std::filesystem::path m_sonic_cppm;
    bool m_is_extern{false};
};

} // namespace cc_abi_gen::parser::helper
