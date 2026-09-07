export module cc_abi_gen_parser:helper_include_finder;

import std;
import cc_utils_cli;

export namespace cc_abi_gen::parser::helper {

class IncludeFinder
{
public:
    IncludeFinder() = default;

    void discover_from_file(
        const std::filesystem::path& path,
        std::vector<std::string>& directories
    ) const
    {
        std::ifstream file_stream(path);
        discover(file_stream, directories);
    }

    std::expected<void, std::string>
    discover_from_command(cc_utils::cli::Command&& cmd, std::vector<std::string>& directories)
    {
        auto result = m_runner.execute(std::move(cmd));

        if (!result) {
            return std::unexpected{result.error()};
        }

        if (!result->success()) {
            return std::unexpected{std::format(
                "Compiler failed to dump includes. Exit code: {}. Error: {}",
                result->get_exit_code(),
                result->get_std_err()
            )};
        }

        std::string output = result->get_std_err() + "\n" + result->get_std_out();
        std::istringstream string_stream(std::move(output));

        discover(string_stream, directories);
        return {};
    }

    void discover(std::istream& stream, std::vector<std::string>& directories) const
    {
        if (!stream.good()) {
            return;
        }

        std::string line;
        bool in_search_list = false;

        while (std::getline(stream, line)) {
            if (line.find("#include <...> search starts here") != std::string::npos) {
                in_search_list = true;
                continue;
            }

            if (line.find("End of search list.") != std::string::npos) {
                break;
            }

            if (in_search_list) {
                const std::string framework_tag = "(framework directory)";
                if (auto pos = line.find(framework_tag); pos != std::string::npos) {
                    line.erase(pos, framework_tag.size());
                }

                auto trimmed_view = line |
                                    std::views::drop_while(
                                        [](unsigned char c)
                                        {
                                            return std::isspace(c);
                                        }
                                    ) |
                                    std::views::reverse |
                                    std::views::drop_while(
                                        [](unsigned char c)
                                        {
                                            return std::isspace(c);
                                        }
                                    ) |
                                    std::views::reverse;

                std::string path(trimmed_view.begin(), trimmed_view.end());

                if (!path.empty()) {
                    directories.push_back(std::move(path));
                }
            }
        }
    }

    const cc_utils::cli::Runner& get_runner() const
    {
        return m_runner;
    }

private:
    cc_utils::cli::Runner m_runner;
};

} // namespace cc_abi_gen::parser::helper
