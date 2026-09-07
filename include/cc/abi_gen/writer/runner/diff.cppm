export module cc_abi_gen_writer:runner_diff;

import std;
import cc_utils_cli;
import :helper_result;

export namespace cc_abi_gen::writer {

class Diff
{
public:
    Diff() = default;
    ~Diff() = default;

    Diff(const Diff&) = delete;
    Diff(Diff&&) = delete;
    Diff& operator=(const Diff&) = delete;
    Diff& operator=(Diff&&) = delete;

    std::expected<helper::DiffResult, std::string>
    compare(const std::filesystem::path& real_path, const std::string& generated_text)
    {
        auto executable = utils::Executable("diff");
        if (!executable.is_found()) {
            return std::unexpected{"diff is not installed or not in PATH."};
        }

        auto cmd = utils::Command(std::move(executable))
                       .arguments(utils::Arguments{"-u", real_path.string(), "-"})
                       .input(std::string(generated_text));

        auto run_result = m_runner.execute(cmd);

        if (!run_result) {
            return std::unexpected{run_result.error()};
        }

        if (run_result->get_exit_code() > 1) {
            return std::unexpected{std::format(
                "diff crashed in {}ms. Error: {}",
                run_result->get_duration().count(),
                run_result->get_std_err()
            )};
        }

        std::println(
            std::format(
                "diff succeeded in {}ms. Exit Code: {}",
                run_result->get_duration().count(),
                run_result->get_exit_code()
            )
        );

        return helper::DiffResult{
            (run_result->get_exit_code() == 0),
            run_result->get_std_out(),
            run_result->get_duration()
        };
    }

    const utils::CommandRunner& get_runner() const
    {
        return m_runner;
    }

private:
    utils::CommandRunner m_runner;
};

} // namespace cc_abi_gen::writer
