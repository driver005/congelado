export module cc_utils_cli:parser_action;

import std;

export namespace cc_utils::cli::parser {

template<typename... Args>
class Action
{
public:
    using Callable = std::move_only_function<void(Args...) const>;

    Action() = default;

    Action(Callable&& func) :
        m_func{std::move(func)}
    {
    }

    Action(const Action&) = delete;
    Action& operator=(const Action&) = delete;
    Action(Action&&) = default;
    Action& operator=(Action&&) = default;

    void set_func(Callable&& func) noexcept
    {
        m_func = std::move(func);
    }

    void execute(Args... args) const
    {
        if (m_func) {
            m_func(std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] Callable& get_func() noexcept
    {
        return m_func;
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
