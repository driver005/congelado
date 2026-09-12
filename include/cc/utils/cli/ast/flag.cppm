export module cc_utils_cli:ast_flag;

import std;

export namespace cc_utils::cli::ast {

using ValidatedValue = std::variant<bool, int, std::string>;

class Flag
{
public:
    Flag() = default;

    Flag(std::string&& name, std::string&& value, bool has_value) noexcept :
        m_name{std::move(name)},
        m_value{std::move(value)},
        m_has_value{has_value}
    {
    }

    Flag(
        std::string&& name,
        std::string&& value,
        bool has_value,
        ValidatedValue&& validated_value
    ) noexcept :
        m_name{std::move(name)},
        m_value{std::move(value)},
        m_has_value{has_value},
        m_validated_value{std::move(validated_value)}
    {
    }

    ~Flag() = default;
    Flag(const Flag&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag(Flag&&) = default;
    Flag& operator=(Flag&&) = default;

    Flag& add_name(std::string&& name) noexcept
    {
        m_names.push_back(std::move(name));
        return *this;
    }

    Flag& add_value(std::string&& value) noexcept {
        m_value = std::move(value);
        return *this;
    }

    Flag& add_has_value(bool has_value) noexcept {
        m_has_value = has_value;
        return *this;
    }

    Flag& 

    void set_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
    }

    void set_value(std::string&& value) noexcept
    {
        m_value = std::move(value);
    }

    void set_has_value(bool has_value) noexcept
    {
        m_has_value = has_value;
    }

    void set_validated_value(ValidatedValue&& value) noexcept
    {
        m_validated_value = std::move(value);
    }

    [[nodiscard]] std::string& get_name() noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] std::string& get_value() noexcept
    {
        return m_value;
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return m_value;
    }

    [[nodiscard]] bool get_has_value() const noexcept
    {
        return m_has_value;
    }

    [[nodiscard]] bool is_boolean() const noexcept
    {
        return !m_has_value;
    }

    [[nodiscard]] std::optional<ValidatedValue>& get_validated_value() noexcept
    {
        return m_validated_value;
    }

    [[nodiscard]] const std::optional<ValidatedValue>& get_validated_value() const noexcept
    {
        return m_validated_value;
    }

private:
    std::string m_name{};
    std::string m_value{};
    bool m_has_value{false};
    std::optional<ValidatedValue> m_validated_value{};
};

} // namespace cc_utils::cli::ast
