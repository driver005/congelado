export module cc_abi_gen_writer:writer;

import std;
import :runner_diff;
import :runner_formater;
import :helper_diff;

export namespace cc_abi_gen::writer {

class Writer
{
public:
    Writer() = default;
    ~Writer() = default;

    Writer(const Writer&) = delete;
    Writer(Writer&&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer& operator=(Writer&&) = delete;

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
    )
    {
        auto formatted = m_formatter.format(rendered_text, repo_root);
        if (!formatted) {
            return std::unexpected{formatted.error()};
        }

        std::error_code error;
        std::filesystem::create_directories(out_path.parent_path(), error);

        std::ofstream out(out_path);
        if (!out) {
            return std::unexpected{"failed to write: " + out_path.string()};
        }
        out << *formatted;

        return {};
    }

    std::expected<helper::DiffResult, std::string> diff(
        const std::string& rendered_text,
        const std::filesystem::path& real_path,
        const std::filesystem::path& repo_root
    )
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

   [[no]] const Formatter& get_formatter() const
    {
        return m_formatter;
    }

    const Diff& get_diff_reporter() const
    {
        return m_diff;
    }

private:
    Formatter m_formatter;
    Diff m_diff;
};

} // namespace cc_abi_gen::writer
