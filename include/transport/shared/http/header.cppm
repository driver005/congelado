export module transport_shared:http_header;

import std;
import transport_error;
import :consts;
import :http_types;

export namespace transport::shared::http {

template <bool IsStatic = false>
class HeaderField {
  public:
    HeaderField() : m_name{}, m_value{""} {}

    HeaderField(Token nameToken, std::string_view value)
        requires(IsStatic == true)
        : m_name{nameToken}, m_value{value} {}

    HeaderField(std::string_view name, std::string_view value)
        requires(IsStatic == false)
        : m_name{std::string(name)}, m_value{value} {
        if (name.empty())
            throw std::runtime_error("Empty name");
    }

    // Accessors
    const auto &get_name() const noexcept { return m_name; }
    const std::string &get_value() const noexcept { return m_value; }

    // Logic for name size
    std::size_t size() const noexcept {
        if constexpr (IsStatic) {
            return sizeof(Token) + m_value.size() + ENTRY_OVERHEAD;
        } else {
            return m_name.size() + m_value.size() + ENTRY_OVERHEAD;
        }
    }

    bool is_empty() const noexcept { return m_value.empty(); }

    // Setters (Only for dynamic version)
    void set_name(std::string name)
        requires(!IsStatic)
    {
        if (name.empty())
            throw std::runtime_error("Empty name");
        m_name = std::move(name);
    }

    void set_value(std::string value) { m_value = std::move(value); }

    bool operator==(const HeaderField &other) const noexcept {

        return m_name == other.m_name && m_value == other.m_value;
    };

  private:
    std::conditional_t<IsStatic, Token, std::string> m_name;
    std::string m_value;
};

// Export a common type for header entries, which can be either static or dynamic
using HeaderEntry = std::variant<std::shared_ptr<HeaderField<true>>, std::shared_ptr<HeaderField<false>>>;
} // namespace transport::shared::http
