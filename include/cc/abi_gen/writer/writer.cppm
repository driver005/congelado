export module cc_abi_gen_writer:writer;

import std;
import :runner_diff;
import :runner_formater;
import :helper_diff;

export namespace cc_abi_gen::writer {

bool should_not_format(const std::filesystem::path& out_path)
{
    constexpr std::array<std::string_view, 4> TARGETS_TO_FORMAT = {"BUILD"};

    std::string filename = out_path.filename().string();
    std::string extension = out_path.extension().string();

    // Check if either the exact filename or the file extension exists in the list
    bool matches_filename =
        std::ranges::find(TARGETS_TO_FORMAT, filename) != TARGETS_TO_FORMAT.end();

    bool matches_extension =
        std::ranges::find(TARGETS_TO_FORMAT, extension) != TARGETS_TO_FORMAT.end();

    return matches_filename || matches_extension;
}

class Writer
{
public:
    Writer() = default;
    ~Writer() = default;

    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer(Writer&&) = default;
    Writer& operator=(Writer&&) = default;

    Writer& add_formater(Formatter&& formatter) noexcept
    {
        m_formatter = std::move(formatter);
        return *this;
    }

    Writer& add_diff(Diff&& differ) noexcept
    {
        m_diff = std::move(differ);
        return *this;
    }

    std::expected<void, std::string> write(
        const std::string& rendered_text,
        const std::filesystem::path& out_path,
        const std::filesystem::path& repo_root
    ) noexcept
    {
        std::string final_text;

        if (should_not_format(out_path.filename())) {
            final_text = rendered_text;
        } else {
            auto formatted = m_formatter.format(rendered_text, repo_root);
            if (!formatted) {
                return std::unexpected{formatted.error()};
            }
            final_text = *formatted;
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

    std::expected<helper::DiffResult, std::string> diff(
        const std::string& rendered_text,
        const std::filesystem::path& real_path,
        const std::filesystem::path& repo_root
    ) noexcept
    {
        auto formatted = m_formatter.format(rendered_text, repo_root);
        if (!formatted) {
            return std::unexpected{formatted.error()};
        }

        return m_diff.compare(real_path, *formatted);
    }

    void set_formater(Formatter&& formatter) noexcept
    {
        m_formatter = std::move(formatter);
    }

    void set_diff(Diff&& differ) noexcept
    {
        m_diff = std::move(differ);
    }

    [[nodiscard]] const Formatter& get_formatter() const noexcept
    {
        return m_formatter;
    }

    [[nodiscard]] const Diff& get_diff_reporter() const noexcept
    {
        return m_diff;
    }

private:
    Formatter m_formatter;
    Diff m_diff;
};

} // namespace cc_abi_gen::writer
