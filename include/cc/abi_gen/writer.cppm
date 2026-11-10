export module cc_abi_gen:writer;

import std;
import cc_utils_cli;

export namespace cc_abi_gen {

// Formats generated text with the repo's clang-format style and writes it to disk.
class Writer
{
public:
    Writer() = default;
    ~Writer() = default;

    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer(Writer&&) = default;
    Writer& operator=(Writer&&) = default;

    std::expected<void, std::string> write(
        const std::string& rendered_text,
        const std::filesystem::path& out_path,
        const std::filesystem::path& repo_root
    )
    {
        std::string final_text = rendered_text;
        if (!is_unformatted(out_path)) {
            auto formatted = format(rendered_text, repo_root);
            if (!formatted) {
                return std::unexpected{std::move(formatted.error())};
            }
            final_text = std::move(*formatted);
        }

        std::error_code error;
        std::filesystem::create_directories(out_path.parent_path(), error);

        std::ofstream out(out_path);
        if (!out) {
            return std::unexpected{"failed to write: " + out_path.string()};
        }
        out << final_text;

        std::println("[cc_abi_gen] wrote: {}", out_path.string());

        return {};
    }

private:
    // BUILD files are not C++; clang-format would mangle them.
    static bool is_unformatted(const std::filesystem::path& out_path)
    {
        return out_path.filename() == "BUILD";
    }

    std::expected<std::string, std::string>
    format(const std::string& source, const std::filesystem::path& repo_root)
    {
        auto executable = cc_utils::cli::Executable("clang-format");
        if (!executable.is_found()) {
            return std::unexpected{"clang-format is not installed or not in PATH."};
        }

        auto command = cc_utils::cli::Command(std::move(executable));
        command
            .add_arguments(
                cc_utils::cli::Arguments{
                    "--assume-filename=.cppm",
                    "-style=file:" + (repo_root / ".clang-format").string(),
                }
            )
            .add_input(std::string(source));

        auto run_result = m_runner.execute(std::move(command));
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

        return run_result->get_std_out();
    }

    cc_utils::cli::Runner m_runner;
};

} // namespace cc_abi_gen
