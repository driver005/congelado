export module cc_utils_cli:command;

import std;
import :arguments;
import :executable;

export namespace cc_utils::cli {

class Command
{
public:
        
    explicit Command(Executable executable) :
        m_executable{std::move(executable)}
    {
    }

    ~Command() = default;
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;
    Command(Command&&) = default;
    Command& operator=(Command&&) = default;

    Command& add_executable(Executable executable)
    {
        m_executable = std::move(executable);
        return *this;
    }

    Command& add_arguments(Arguments&& args)
    {
        m_args = std::move(args);
        return *this;
    }

    Command& add_input(std::string&& text)
    {
        m_stdin_text = std::move(text);
        return *this;
    }

    Command& add_input_chunk(std::string_view chunk)
    {
        m_stdin_text.append(chunk);
        return *this;
    }

    void set_executable(Executable&& executable)
    {
        m_executable = std::move(executable);
    }

    void set_arguments(Arguments&& args)
    {
        m_args = std::move(args);
    }

    void set_input(std::string&& input)
    {
        m_stdin_text = std::move(input);
    }

    void append_input(std::string&& chunk)
    {
        m_stdin_text += std::move(chunk);
    }

    const Executable& get_executable() const
    {
        return m_executable;
    }

    const Arguments& get_arguments() const
    {
        return m_args;
    }

    const std::string& get_stdin_text() const
    {
        return m_stdin_text;
    }

    std::vector<char*> to_c_args() const
    {
        // Pass the resolved path (or the name if resolution failed) to argv[0]
        const std::string& exe_str =
            m_executable.is_found() ? m_executable.get_path() : m_executable.get_name();
        return m_args.get_c_args(exe_str);
    }

private:
    Executable m_executable;
    Arguments m_args;
    std::string m_stdin_text;
};

} // namespace cc_utils::cli
