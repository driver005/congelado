export module cc_abi_gen_parser:domain_paths;

import std;

export namespace cc_abi_gen {

// Resolves one pilot domain's header path and its two generated-file paths under a given output
// root. For the manual `bazel run` path (no --out-dir), the caller passes repo_root/include/cc/abi
// so this regenerates the real, checked-in files in place; for the genrule-driven build-time
// path, the caller passes the genrule's own output directory instead, so nothing outside that
// directory is ever touched.
class DomainPaths
{
public:
    DomainPaths(
        const std::string &domain,
        const std::filesystem::path &repo_root,
        const std::filesystem::path &output_root)
    {
        m_header = repo_root / "include/c/extern" / domain / (domain + ".h");
        m_builder_cppm = output_root / "builder" / domain / (domain + ".cppm");
        m_sonic_cppm = output_root / "sonic" / domain / (domain + ".cppm");
    }

    std::filesystem::path m_header;
    std::filesystem::path m_builder_cppm;
    std::filesystem::path m_sonic_cppm;
};

} // namespace cc_abi_gen
