export module cc_utils_cli:parser_helper;

import std;

export namespace cc_utils::cli::parser {

enum class FlagType
{
    Boolean,
    String,
    Int
};

class FlagConfig
{
public:
    FlagConfig() = default;

    FlagConfig(bool required, bool hidden) noexcept :
        m_required{required},
        m_hidden{hidden}
    {
    }

    FlagConfig(
        std::optional<std::string>&& default_value,
        std::optional<int>&& min,
        std::optional<int>&& max,
        bool required,
        bool hidden,
        std::optional<std::string>&& env_var,
        std::vector<std::string>&& allowed_values,
        std::vector<std::string>&& requires_flags,
        std::vector<std::string>&& conflicts_with
    ) noexcept :
        m_default_value{std::move(default_value)},
        m_min{std::move(min)},
        m_max{std::move(max)},
        m_required{required},
        m_hidden{hidden},
        m_env_var{std::move(env_var)},
        m_allowed_values{std::move(allowed_values)},
        m_requires_flags{std::move(requires_flags)},
        m_conflicts_with{std::move(conflicts_with)}
    {
    }

    ~FlagConfig() = default;
    FlagConfig(const FlagConfig&) = delete;
    FlagConfig& operator=(const FlagConfig&) = delete;
    FlagConfig(FlagConfig&&) = default;
    FlagConfig& operator=(FlagConfig&&) = default;

    FlagConfig& add_default_value(std::string&& value)
    {
        m_default_value = std::move(value);
        return *this;
    }

    FlagConfig& add_min(std::string&& min)
    {
        m_min = std::move(min);
        return *this;
    }

    FlagConfig& add_max(std::string&& max)
    {
        m_max = std::move(max);
        return *this;
    }

    FlagConfig& add_required(bool required)
    {
        m_required = required;
        return *this;
    }

    FlagConfig& add_hidden(bool hidden)
    {
        m_hidden = hidden;
        return *this;
    }

    FlagConfig& add_env_var(std::string&& env_var)
    {
        m_env_var = std::move(env_var);
        return *this;
    }

    FlagConfig& add_allowed_value(std::string&& value)
    {
        m_allowed_values.push_back(std::move(value));
        return *this;
    }

    FlagConfig& add_requires_flag(std::string&& value)
    {
        m_requires_flags.push_back(std::move(value));
        return *this;
    }

    FlagConfig& add_conflicts_with(std::string&& value)
    {
        m_conflicts_with.push_back(std::move(value));
        return *this;
    }

    [[nodiscard]] bool is_required() const noexcept
    {
        return m_required;
    }

    [[nodiscard]] bool is_hidden() const noexcept
    {
        return m_hidden;
    }

    void set_default_value(std::string&& value) noexcept
    {
        m_default_value = std::move(value);
    }

    void set_min(int value) noexcept
    {
        m_min = value;
    }

    void set_max(int value) noexcept
    {
        m_max = value;
    }

    void set_required(bool value) noexcept
    {
        m_required = value;
    }

    void set_hidden(bool value) noexcept
    {
        m_hidden = value;
    }

    void set_env_var(std::string&& value) noexcept
    {
        m_env_var = std::move(value);
    }

    void append_allowed_value(std::string&& value) noexcept
    {
        m_allowed_values.push_back(std::move(value));
    }

    void append_requires_flag(std::string&& value) noexcept
    {
        m_requires_flags.push_back(std::move(value));
    }

    void append_coflicts_with(std::string&& value) noexcept
    {
        m_conflicts_with.push_back(std::move(value));
    }

    [[nodiscard]] std::optional<std::string>& get_default_value() noexcept
    {
        return m_default_value;
    }

    [[nodiscard]] std::optional<int>& get_min() noexcept
    {
        return m_min;
    }

    [[nodiscard]] std::optional<int>& get_max() noexcept
    {
        return m_max;
    }

    [[nodiscard]] std::optional<std::string>& get_env_var() noexcept
    {
        return m_env_var;
    }

    [[nodiscard]] std::span<std::string>& get_allowed_values() noexcept
    {
        return m_allowed_values;
    }

    [[nodiscard]] std::span<std::string>& get_requires_flags() noexcept
    {
        return m_requires_flags;
    }

    [[nodiscard]] std::span<std::string>& get_conflicts_with() noexcept
    {
        return m_conflicts_with;
    }

    [[nodiscard]] const std::optional<std::string>& get_default_value() const noexcept
    {
        return m_default_value;
    }

    [[nodiscard]] const std::optional<const int>& get_min() const noexcept
    {
        return m_min;
    }

    [[nodiscard]] const std::optional<const int>& get_max() const noexcept
    {
        return m_max;
    }

    [[nodiscard]] const std::optional<const std::string>& get_env_var() const noexcept
    {
        return m_env_var;
    }

    [[nodiscard]] std::span<const std::string> get_allowed_values() const noexcept
    {
        return m_allowed_values;
    }

    [[nodiscard]]  std::span<const std::string>  get_requires_flags() const noexcept
    {
        return m_requires_flags;
    }

    [[nodiscard]]  std::span<const std::string>  get_conflicts_with() const noexcept
    {
        return m_conflicts_with;
    }

private:
    std::optional<std::string> m_default_value{};
    std::optional<int> m_min{};
    std::optional<int> m_max{};
    bool m_required{false};
    bool m_hidden{false};
    std::optional<std::string> m_env_var{};
    std::vector<std::string> m_allowed_values{};
    std::vector<std::string> m_requires_flags{};
    std::vector<std::string> m_conflicts_with{};
};

class ParserOptions
{
public:
    ParserOptions() = default;

    ParserOptions(bool allow_unrecognized, bool auto_help) noexcept :
        m_allow_unrecognized{allow_unrecognized},
        m_auto_help{auto_help}
    {
    }

    ParserOptions(
        bool allow_unrecognized,
        bool auto_help,
        std::optional<std::string>&& version
    ) noexcept :
        m_allow_unrecognized{allow_unrecognized},
        m_auto_help{auto_help},
        m_version{std::move(version)}
    {
    }

    ~ParserOptions() = default;
    ParserOptions(const ParserOptions&) = delete;
    ParserOptions& operator=(const ParserOptions&) = delete;
    ParserOptions& operator=(ParserOptions&&) = default;
    ParserOptions(ParserOptions&&) = default;

    ParserOption& add_allow_unrecognized() noexcept
    {
        m_allow_unrecognized = true;
        return *this;
    }

    ParserOption& add_auto_help() noexcept
    {
        m_auto_help = true;
        return *this;
    }

    ParserOptions& add_version(std::string&& value) noexcept
    {
        m_version = std::move(value);
        return *this;
    }

    void set_allow_unrecognized(bool value) noexcept
    {
        m_allow_unrecognized = value;
    }

    void set_auto_help(bool value) noexcept
    {
        m_auto_help = value;
    }

    void set_version(std::string&& value) noexcept
    {
        m_version = std::move(value);
    }

    [[nodiscard]] bool get_allows_unrecognized() const noexcept
    {
        return m_allow_unrecognized;
    }

    [[nodiscard]] bool get_has_auto_help() const noexcept
    {
        return m_auto_help;
    }

    [[nodiscard]] const std::optional<const std::string>& get_version() const noexcept
    {
        return m_version;
    }

private:
    bool m_allow_unrecognized{false};
    bool m_auto_help{true};
    std::optional<std::string> m_version{};
};
} // namespace cc_utils::cli::parser
