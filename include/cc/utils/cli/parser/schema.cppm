export module cc_utils_cli:parser_schema;

import std;
import :parser_option;
import :parser_flag;
import :parser_action;
import :ast_invocation;

export namespace cc_utils::cli::parser {

// One command a Parser accepts (e.g. "generate") and the `--flag` names it allows.
class Schema
{
public:
    Schema() = default;

    explicit Schema(std::string&& name) noexcept :
        m_name{std::move(name)}
    {
    }

    Schema(
        std::string&& name,
        std::vector<Option>&& options,
        std::vector<Flag>&& flags,
        Action<std::span<const Flag>>&& action
    ) noexcept :
        m_name{std::move(name)},
        m_options{std::move(options)},
        m_flags{std::move(flags)},
        m_action{std::move(action)}
    {
    }
        ~Schema() = default;
        

    Schema& add_name(std::string&& value) noexcept
    {
        m_name = std::move(value);
        return *this;
    }

    Schema& add_option(Option&& opt)
    {
        m_options.push_back(std::move(opt));
        return *this;
    }

    Schema& add_flag(Flag&& flg)
    {
        m_flags.push_back(std::move(flg));
        return *this;
    }

    Schema& add_action(Action&& act) noexcept
    {
        m_action = std::move(act);
        return *this;
    }

    void set_name(std::string&& value) noexcept
    {
        m_name = std::move(value);
    }

    void append_option(Option&& opt) noexcept
    {
        m_options.push_back(std::move(opt));
    }

    void append_flag(Flag&& flag) noexcept
    {
        m_flags.push_back(std::move(flag));
    }

    template<typename Callable>
    void set_action(Callable&& act)
    {
        m_action = std::forward<Callable>(act);
    }

    [[nodiscard]] std::expected<void, std::string>
    validate(ast::Invocation& ast_invoc, bool allow_unrecognized = false) const
    {
        auto result = validate_flags(ast_invoc.get_global_flags(), allow_unrecognized);
        if (!result) {
            return result;
        }

        for (auto& ast_cmd: ast_invoc.get_commands()) {
            auto opt_ref = get_option(ast_cmd.get_name(), allow_unrecognized);

            if (!opt_ref) {
                if (!allow_unrecognized) {
                    return std::unexpected(
                        std::format("Unknown command provided: '{}'", ast_cmd.get_name())
                    );
                }
                continue;
            }

            if (auto result = opt_ref->get().validate(ast_cmd, allow_unrecognized); !result) {
                return result;
            }
        }

        return {};
    }

    [[nodiscard]] std::expected<void, std::string>
    validate(ast::Command& ast_cmd, bool allow_unrecognized = false) const
    {
        auto result = validate_flags(ast_cmd.get_flags(), allow_unrecognized);
        if (!result) {
            return result;
        }

        for (auto& ast_sub: ast_cmd.get_subcommands()) {
            auto opt_ref = get_option(ast_sub.get_name(), allow_unrecognized);

            if (!opt_ref) {
                if (!allow_unrecognized) {
                    return std::unexpected(
                        std::format("Unknown subcommand provided: '{}'", ast_sub.get_name())
                    );
                }
                continue;
            }

            if (auto result = opt_ref->get().validate(ast_sub, allow_unrecognized); !result) {
                return result;
            }
        }

        return {};
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const Option>>
    get_option(std::string_view option_name, bool allow_unrecognized) const noexcept
    {
        auto projection = [](const Option& option) -> const std::string&
        {
            return option.get_name();
        };

        auto it = std::ranges::find(m_options, option_name, projection);
        if (it != m_options.end()) {
            return std::cref(*it);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const Flag>>
    get_flag(std::string_view flag_name) const noexcept
    {
        auto projection = [](const Flag& flag) -> const std::string&
        {
            return flag.get_name();
        };

        auto it = std::ranges::find(m_flags, flag_name, projection);
        if (it != m_flags.end()) {
            return std::cref(*it);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string& get_name() noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] bool accepts_option(std::string_view option_name) const noexcept
    {
        auto projection = [](const Option& option) -> const std::string&
        {
            return option.get_name();
        };

        return std::ranges::contains(m_options, option_name, projection);
    }

    [[nodiscard]] bool accepts_flag(std::string_view flag_name) const noexcept
    {
        auto projection = [](const Flag& flag) -> const std::string&
        {
            return flag.get_name();
        };

        return std::ranges::contains(m_flags, flag_name, projection);
    }

    [[nodiscard]] std::span<Flag> get_flags() noexcept
    {
        return m_flags;
    }

    [[nodiscard]] std::span<const Flag> get_flags() const noexcept
    {
        return m_flags;
    }

    [[nodiscard]] std::span<Option> get_options() noexcept
    {
        return m_options;
    }

    [[nodiscard]] std::span<const Option> get_options() const noexcept
    {
        return m_options;
    }

    [[nodiscard]] Action<std::span<const Flag>>& get_action() noexcept
    {
        return m_action;
    }

    [[nodiscard]] const Action<std::span<const Flag>>& get_action() const noexcept
    {
        return m_action;
    }

private:
    [[nodiscard]] std::expected<void, std::string>
    validate_flags(std::span<ast::Flag> ast_flags, bool allow_unrecognized) const
    {
        for (auto& ast_flag: ast_flags) {
            if (auto flag_ref = get_flag(ast_flag.get_name())) {
                if (auto result = flag_ref->get().validate(ast_flag); !result) {
                    return result;
                }
                continue;
            }

            if (!allow_unrecognized) {
                return std::unexpected(
                    std::format("Unknown flag provided: '--{}'", ast_flag.get_name())
                );
            }
        }

        return {};
    }

    std::string m_name{};
    std::vector<Option> m_options{};
    std::vector<Flag> m_flags{};
    Action<std::span<const Flag>> m_action{};
};

} // namespace cc_utils::cli::parser
