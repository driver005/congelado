export module cc_abi_gen:cli_options;

import std;

export namespace cc_abi_gen {

// Parsed command-line flags for one cc_abi_gen invocation.
class CliOptions
{
public:
    std::optional<std::string> m_folder;
    std::optional<std::string> m_namespace_name;
    std::optional<std::string> m_out_dir;
    std::optional<std::string> m_repo_root;
};

} // namespace cc_abi_gen
