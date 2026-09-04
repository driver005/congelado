export module cc_abi_gen_writer:generated_file_writer;

import std;
import :diff_result;
import :clang_format_runner;
import :diff_reporter;

export namespace cc_abi_gen {

// Formats generated source (via ClangFormatRunner) and either writes it to disk or diffs it
// against a real, checked-in file (via DiffReporter) — the single entry point the writer tier
// exposes to the CLI layer.
class GeneratedFileWriter
{
public:
    std::expected<void, std::string> write(
        const std::string &rendered_text,
        const std::filesystem::path &out_path,
        const std::filesystem::path &repo_root)
    {
        auto formatted = m_formatter.format(rendered_text, repo_root);
        if (!formatted) {

            return std::unexpected{formatted.error()};
        }

        // A genrule-driven out-dir starts empty each build — the domain subdirectories under it
        // don't exist yet, so they're created here rather than assumed to be there already (the
        // checked-in include/cc/abi/... tree this also writes to already has them, so this is a
        // no-op there).
        std::error_code error;
        std::filesystem::create_directories(out_path.parent_path(), error);

        std::ofstream out(out_path);
        if (!out) {

            return std::unexpected{"failed to write: " + out_path.string()};
        }
        out << *formatted;

        return {};
    }

    std::expected<DiffResult, std::string> diff(
        const std::string &rendered_text,
        const std::filesystem::path &real_path,
        const std::filesystem::path &repo_root)
    {
        auto formatted = m_formatter.format(rendered_text, repo_root);
        if (!formatted) {

            return std::unexpected{formatted.error()};
        }

        return m_diff_reporter.compare(real_path, *formatted);
    }

private:
    ClangFormatRunner m_formatter;
    DiffReporter m_diff_reporter;
};

} // namespace cc_abi_gen
