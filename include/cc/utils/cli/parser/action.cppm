export module cc_utils_cli_parser:parser_action;

import std;

export namespace cc_utils::cli::parser {

class Action
{
public:
    using Callable = std::move_only_function<void() const>;

    Action() = default;

    Action(Callable&& func) :
        m_func{std::move(func)}
    {
    }

    ~Action() = default;
    Action(const Action&) = delete;
    Action& operator=(const Action&) = delete;
    Action(Action&&) = default;
    Action& operator=(Action&&) = default;

    Action& add_function(Callable&& func) noexcept
    {
        m_func = std::move(func);
        return *this;
    }

    void execute() const noexcept
    {
        if (m_func) {
            m_func();
        }
    }

    void set_func(Callable&& func) noexcept
    {
        m_func = std::move(func);
    }

    [[nodiscard]] const Callable& get_func() const noexcept
    {
        return m_func;
    }

    explicit operator bool() const noexcept
    {
        return m_func != nullptr;
    }

private:
    Callable m_func{};
};

} // namespace cc_utils::cli::parser
