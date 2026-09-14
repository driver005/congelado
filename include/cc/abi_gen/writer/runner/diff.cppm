export module cc_abi_gen_writer:runner_diff;

import std;
import cc_utils_cli;
import :helper_diff;

export namespace cc_abi_gen::writer {

class Diff
{
public:
    Diff() = default;
    ~Diff() = default;

    Diff(const Diff&) = delete;
    Diff& operator=(const Diff&) = delete;
    Diff(Diff&&) = default;
    Diff& operator=(Diff&&) = default;

    Diff& add_runner(cc_utils::cli::Runner&& runner) noexcept
    {
        m_runner = std::move(runner);
        return *this;
    }

    std::expected<helper::DiffResult, std::string>
    compare(const std::filesystem::path& real_path, const std::string& generated_text) noexcept
    {
        auto executable = cc_utils::cli::Executable("diff");
        if (!executable.is_found()) {
            return std::unexpected{"diff is not installed or not in PATH."};
        }

        auto cmd = cc_utils::cli::Command(std::move(executable))
                       .arguments(cc_utils::cli::Arguments{"-u", real_path.string(), "-"})
                       .input(std::string(generated_text));

        auto run_result = m_runner.execute(std::move(cmd));

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
            "diff succeeded in {}ms. Exit Code: {}",
            run_result->get_duration().count(),
            run_result->get_exit_code()
        );

        auto code = (run_result->get_exit_code() == 0);
        return helper::DiffResult{
            code,
            std::move(run_result->get_std_out()),
            std::move(run_result->get_duration())
        };
    }

    void set_runner(cc_utils::cli::Runner&& runner) noexcept
    {
        m_runner = std::move(runner);
    }

    cc_utils::cli::Runner& get_runner() noexcept
    {
        return m_runner;
    }

    const cc_utils::cli::Runner& get_runner() const noexcept
    {
        return m_runner;
    }

private:
    cc_utils::cli::Runner m_runner;
};

} // namespace cc_abi_gen::writer
