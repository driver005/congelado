export module cc_utils_cli:parser_option;

import std;
import :parser_flag;
import :ast_command;

export namespace cc_utils::cli::parser {

class Option
{
public:
    Option() = default;

    Option(std::string&& name, std::string&& description) noexcept :
        m_name(std::move(name)),
        m_description(std::move(description))
    {
    }

    Option(
        std::string&& name,
        std::string&& description,
        std::vector<Flag>&& flags,
        std::vector<Option>&& subcommands,
        std::function<void(const ast::Command&)>&& action
    ) noexcept :
        m_name{std::move(name)},
        m_description{std::move(description)},
        m_flags{std::move(flags)},
        m_subcommands{std::move(subcommands)},
        m_action{std::move(action)}
    {
    }

    ~Option() = default;
    Option(const Option&) = delete;
    Option& operator=(const Option&) = delete;
    Option(Option&&) = default;
    Option& operator=(Option&&) = default;

    Option& add_name(std::string&& name)
    {
        m_name = std::move(name);
        return *this;
    }

    Option& add_description(std::string&& description)
    {
        m_description = std::move(description);
        return *this;
    }

    Option& add_flag(Flag&& option)
    {
        m_flags.push_back(std::move(option));
        return *this;
    }

    Option& add_subcommand(Option&& cmd)
    {
        m_subcommands.push_back(std::move(cmd));
        return *this;
    }

    Option& add_action(std::function<void(const ast::Command&)>&& cb)
    {
        m_action = std::move(cb);
        return *this;
    }

    void set_name(std::string&& value) noexcept
    {
        m_name = std::move(value);
    }

    void set_description(std::string&& value) noexcept
    {
        m_description = std::move(value);
    }

    void append_flag(Option&& opt) noexcept
    {
        m_flags.push_back(std::move(opt));
    }

    void append_subcommand(Command&& cmd) noexcept
    {
        m_subcommands.push_back(std::move(cmd));
    }

    void set_action(std::function<void(const ast::Command&)>&& cb) noexcept
    {
        m_action = std::move(cb);
    }

    [[nodiscard]] std::expected<void, std::string>
    validate(ast::Command& parser_cmd, bool allow_unrecognized) const
    {
        for (auto& ast_flag: parser_cmd.get_flags()) {
            auto it = std::ranges::find_if(
                m_flags,
                [&](const auto& opt)
                {
                    return opt.get_name() == ast_flag.get_name();
                }
            );

            if (!allow_unrecognized) {
                if (it == m_flags.end()) {
                    return std::unexpected(
                        std::format("Unknown flag provided: '--{}'", ast_flag.get_name())
                    );
                }
            }

            // Trigger the schema's flag validation, which writes the typed data back into
            // ast_flag
            if (auto result = it->validate(ast_flag); !result) {
                return result;
            }
        }

        // Recursively validate subcommands
        for (auto& ast_sub_cmd: parser_cmd.get_subcommands()) {
            auto it = std::ranges::find_if(
                m_subcommands,
                [&](const auto& sub)
                {
                    return sub.get_name() == ast_sub_cmd.get_name();
                }
            );

            if (it == m_subcommands.end()) {
                return std::unexpected(
                    std::format("Unknown subcommand provided: '{}'", ast_sub_cmd.get_name())
                );
            }

            // Recursive call down the tree!
            if (auto result = it->validate(ast_sub_cmd, allow_unrecognized); !result) {
                return result;
            }
        }

        return {};
    }

    void execute(const ast::Command& parser_cmd) const
    {
        if (m_action) {
            m_action(parser_cmd);
        }
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

    [[nodiscard]] bool accepts_flag(std::string_view flag_name) const noexcept
    {
        auto projection = [](const Flag& flag) -> const std::string&
        {
            return flag.get_name();
        };

        return std::ranges::contains(m_flags, flag_name, projection);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    [[nodiscard]] const std::span<const Flag> get_flags() const noexcept
    {
        return m_flags;
    }

    [[nodiscard]] const std::span<const Option> get_subcommands() const noexcept
    {
        return m_subcommands;
    }

    [[nodiscard]] const std::function<void(const ast::Command&)>& get_action() const noexcept
    {
        return m_action;
    }

private:
    std::string m_name{};
    std::string m_description{};
    std::vector<Flag> m_flags{};
    std::vector<Option> m_subcommands{};
    std::function<void(const ast::Command&)> m_action{};
};

} // namespace cc_utils::cli::parser
