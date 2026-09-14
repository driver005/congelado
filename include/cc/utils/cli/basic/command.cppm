export module cc_utils_cli:basic_command;

import std;
import :basic_argument;
import :executable;

export namespace cc_utils::cli {

class Command
{
public:
    explicit Command(Executable&& executable) noexcept :
        m_executable{std::move(executable)}
    {
    }

    ~Command() = default;
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;
    Command(Command&&) = default;
    Command& operator=(Command&&) = default;

    Command& add_executable(Executable&& executable) noexcept
    {
        m_executable = std::move(executable);
        return *this;
    }

    Command& add_argument(Argument&& arg) noexcept
    {
        m_args.emplace_back(std::move(arg));
        return *this;
    }

    Command& add_stdin(std::string&& text) noexcept
    {
        m_stdin_text = std::move(text);
        return *this;
    }

    void set_arguments(Arguments&& args) noexcept
    {
        m_args = std::move(args);
    }

    void set_input(std::string&& text) noexcept
    {
        m_stdin_text = std::move(text);
    }

    void append_input(std::string_view chunk)
    {
        m_stdin_text.append(chunk);
    }

    [[nodiscard]] const Executable& get_executable() const noexcept
    {
        return m_executable;
    }

    [[nodiscard]] const Arguments& get_arguments() const noexcept
    {
        return m_args;
    }

    [[nodiscard]] const std::string& get_stdin_text() const noexcept
    {
        return m_stdin_text;
    }

    [[nodiscard]] std::vector<char*> to_c_args() const
    {
        // Pass the resolved path (or the name if resolution failed) to argv[0]
        const std::string& exe_str =
            m_executable.is_found() ? m_executable.get_path() : m_executable.get_name();
        return m_args.get_c_args(exe_str);
    }

private:
    Executable m_executable;
    Arguments m_args{};
    std::string m_stdin_text{};
};

} // namespace cc_utils::cli
