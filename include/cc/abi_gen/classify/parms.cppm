export module cc_abi_gen_classify:parms;

import std;

export namespace cc_abi_gen::classify {

class Param
{
public:
    Param() = default;

    Param(
        std::string type,
        std::string name,
        std::optional<std::string> defaultValue = std::nullopt,
        std::string description = ""
    ) :
        m_type(std::move(type)),
        m_name(std::move(name)),
        m_default_value(std::move(defaultValue)),
        m_description(std::move(description))
    {
    }

    ~Param() = default;

    Param(const Param& other) = delete;
    Param& operator=(const Param& other) = delete;
    Param(Param&& other) noexcept = default;
    Param& operator=(Param&& other) noexcept = default;

    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::optional<std::string>& get_default_value() const noexcept
    {
        return m_default_value;
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    [[nodiscard]] std::string to_string(bool includeDefault = true) const
    {
        // Function pointer: the name goes inside the declarator, `void (*callback)(int)`.
        constexpr std::string_view POINTER_MARKER = "(*)";

        if (auto position = std::string_view{m_type}.find(POINTER_MARKER);
            position != std::string_view::npos) {
            std::string declaration{m_type};
            declaration.insert(position + 2, m_name);

            return declaration;
        }

        if (includeDefault && m_default_value.has_value()) {
            return std::format("{} {} = {}", m_type, m_name, *m_default_value);
        }
        return std::format("{} {}", m_type, m_name);
    }

private:
    std::string m_type;
    std::string m_name;
    std::optional<std::string> m_default_value;
    // Used for Doxygen @param documentation
    std::string m_description;
};

} // namespace cc_abi_gen::classify
