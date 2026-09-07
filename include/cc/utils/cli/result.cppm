export module cc_utils_cli:result;

import std;

export namespace cc_utils::cli {

class Result
{
public:
    Result(
        int exit_code,
        bool exited_normally,
        int term_signal,
        std::string&& std_out,
        std::string&& std_err,
        std::chrono::milliseconds duration
    ) :
        m_exit_code{exit_code},
        m_exited_normally{exited_normally},
        m_term_signal{term_signal},
        m_std_out{std::move(std_out)},
        m_std_err{std::move(std_err)},
        m_duration{duration}
    {
    }

    int get_exit_code() const
    {
        return m_exit_code;
    }

    bool get_exited_normally() const
    {
        return m_exited_normally;
    }

    int get_term_signal() const
    {
        return m_term_signal;
    }

    const std::string& get_std_out() const
    {
        return m_std_out;
    }

    const std::string& get_std_err() const
    {
        return m_std_err;
    }

    std::chrono::milliseconds get_duration() const
    {
        return m_duration;
    }

    bool success() const
    {
        return m_exited_normally && m_exit_code == 0;
    }

private:
    int m_exit_code;
    bool m_exited_normally;
    int m_term_signal;
    std::string m_std_out;
    std::string m_std_err;
    std::chrono::milliseconds m_duration;
};

} // namespace cc_utils::cli
