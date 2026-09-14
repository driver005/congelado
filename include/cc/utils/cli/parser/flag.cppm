export module cc_utils_cli:parser_flag;

import std;
import :parser_helper;
import :ast_flag;

export namespace cc_utils::cli::parser {

class Flag
{
public:
    Flag() = default;

    Flag(std::string&& name, std::string&& description, FlagType type, FlagConfig&& config) noexcept
        :
        m_name{std::move(name)},
        m_description{std::move(description)},
        m_type{type},
        m_config{std::move(config)}
    {
    }

    ~Flag() = default;
    Flag(const Flag&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag(Flag&&) = default;
    Flag& operator=(Flag&&) = default;

    Flag& add_name(std::string&& name)
    {
        m_name = std::move(name);
        return *this;
    }

    Flag& add_description(std::string&& description)
    {
        m_description = std::move(description);
        return *this;
    }

    Flag& add_type(FlagType&& type)
    {
        return *this;
    }

    Flag& add_config(FlagConfig&& config)
    {
        m_config = std::move(config);
        return *this;
    }

    [[nodiscard]] std::expected<void, std::string> validate(ast::Flag& parsed_flag) const noexcept
    {
        switch (m_type) {
            case FlagType::Boolean:
                {
                    if (!parsed_flag.is_boolean()) {
                        return std::unexpected(
                            std::format("Flag '--{}' is a boolean and cannot take a value.", m_name)
                        );
                    }

                    parsed_flag.set_validated_value(true);
                    break;
                }

            case FlagType::String:
                {
                    if (parsed_flag.is_boolean()) {
                        return std::unexpected(
                            std::format("Flag '--{}' requires a string value.", m_name)
                        );
                    }

                    const std::string& val = parsed_flag.get_value();

                    // Enforce string length boundaries using getters
                    if (m_config.get_min().has_value() && std::ssize(val) < *m_config.get_min()) {
                        return std::unexpected(
                            std::format(
                                "Flag '--{}' length is shorter than the minimum ({} chars).",
                                m_name,
                                *m_config.get_min()
                            )
                        );
                    }
                    if (m_config.get_max().has_value() && std::ssize(val) > *m_config.get_max()) {
                        return std::unexpected(
                            std::format(
                                "Flag '--{}' length is longer than the maximum ({} chars).",
                                m_name,
                                *m_config.get_max()
                            )
                        );
                    }

                    // Enforce specific allowed values using getters
                    if (!m_config.get_allowed_values().empty()) {
                        auto it = std::ranges::find(m_config.get_allowed_values(), val);
                        if (it == m_config.get_allowed_values().end()) {
                            return std::unexpected(
                                std::format(
                                    "Flag '--{}' received invalid choice '{}'.",
                                    m_name,
                                    val
                                )
                            );
                        }
                    }

                    parsed_flag.set_validated_value(val);
                    break;
                }

            case FlagType::Int:
                {
                    if (parsed_flag.is_boolean()) {
                        return std::unexpected(
                            std::format("Flag '--{}' requires an integer value.", m_name)
                        );
                    }

                    const std::string& val = parsed_flag.get_value();
                    int parsed_int{};
                    auto [ptr, ec] =
                        std::from_chars(val.data(), val.data() + val.size(), parsed_int);

                    if (ec != std::errc{} || ptr != val.data() + val.size()) {
                        return std::unexpected(
                            std::format(
                                "Flag '--{}' expects a valid integer, but got '{}'.",
                                m_name,
                                val
                            )
                        );
                    }

                    // Enforce numeric value boundaries using getters
                    if (m_config.get_min().has_value() && parsed_int < *m_config.get_min()) {
                        return std::unexpected(
                            std::format(
                                "Flag '--{}' value {} is below the minimum allowed ({}).",
                                m_name,
                                parsed_int,
                                *m_config.get_min()
                            )
                        );
                    }
                    if (m_config.get_max().has_value() && parsed_int > *m_config.get_max()) {
                        return std::unexpected(
                            std::format(
                                "Flag '--{}' value {} is above the maximum allowed ({}).",
                                m_name,
                                parsed_int,
                                *m_config.get_max()
                            )
                        );
                    }

                    parsed_flag.set_validated_value(parsed_int);
                    break;
                }
        }

        return {};
    }

    [[nodiscard]] std::optional<std::string_view> has_default_value() noexcept
    {
        if (m_config.get_default_value()) {
            return *m_config.get_default_value();
        }
        return std::nullopt;
    }

    void set_name(std::string&& value) noexcept
    {
        m_name = std::move(value);
    }

    void set_description(std::string&& value) noexcept
    {
        m_description = std::move(value);
    }

    void set_type(FlagType&& value) noexcept
    {
        m_type = std::move(value);
    }

    void set_config(FlagConfig&& value) noexcept
    {
        m_config = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    [[nodiscard]] FlagType get_type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const FlagConfig& get_config() const noexcept
    {
        return m_config;
    }

private:
    std::string m_name{};
    std::string m_description{};
    FlagType m_type{FlagType::Boolean};
    FlagConfig m_config{};
};

} // namespace cc_utils::cli::parser
