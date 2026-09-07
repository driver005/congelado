export module cc_abi_gen_writer:runner_formater;

import std;
import cc_utils_cli;

export namespace cc_abi_gen::writer {

class Formatter
{
public:
    Formatter() = default;
    ~Formatter() = default;

    Formatter(const Formatter&) = delete;
    Formatter(Formatter&&) = delete;
    Formatter& operator=(const Formatter&) = delete;
    Formatter& operator=(Formatter&&) = delete;

    std::expected<std::string, std::string>
    format(const std::string& source, const std::filesystem::path& repo_root)
    {
        auto executable = cc_utils::cli::Executable("clang-format");
        if (!executable.is_found()) {
            return std::unexpected{"clang-format is not installed or not in PATH."};
        }

        auto cmd = cc_utils::cli::Command(std::move(executable))
                       .arguments(
                           cc_utils::cli::Arguments{
                               "--assume-filename=.cppm",
                               "-style=file:" + (repo_root / ".clang-format").string()
                           }
                       )
                       .input(std::string(source));

        auto run_result = m_runner.execute(std::move(cmd));
        if (!run_result) {
            return std::unexpected{run_result.error()};
        }

        if (!run_result->success()) {
            return std::unexpected{std::format(
                "clang-format failed in {}ms. Exit Code: {}. Error: {}",
                run_result->get_duration().count(),
                run_result->get_exit_code(),
                run_result->get_std_err()
            )};
        }

        std::println(
            "clang-format succeeded in {}ms. Exit Code: {}",
            run_result->get_duration().count(),
            run_result->get_exit_code()
        );

        return run_result->get_std_out();
    }

    const cc_utils::cli::Runner& get_runner() const
    {
        return m_runner;
    }

private:
    cc_utils::cli::Runner m_runner;
};

} // namespace cc_abi_gen::writer
