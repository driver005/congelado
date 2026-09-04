export module cc_abi_gen:cli_options;

import std;

export namespace cc_abi_gen {

// Parsed command-line flags for one cc_abi_gen invocation.
class CliOptions
{
public:
    bool m_pilot = false;
    std::optional<std::string> m_tier;
    std::optional<std::string> m_domain;
    std::optional<std::string> m_header;
    std::optional<std::string> m_out;
    std::optional<std::string> m_out_dir;
    std::optional<std::string> m_repo_root;
};

} // namespace cc_abi_gen
