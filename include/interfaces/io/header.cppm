export module interfaces:io_header;

import std;
import :consts;
import :io_types;

export namespace interfaces::io {

template <bool IsStatic = false>
class HeaderField {
  public:
    /**
     * @brief Default ctor — empty name, empty value. Blank slate, nothing loaded in yet, just
     * vibes and zeroed-out members.
     */
    HeaderField() : m_name{} {}

    /**
     * @brief Builds a static-flavored header field from a known `Token` — only exists when
     * `IsStatic` is true, so you're locked into an interned token instead of a free-string
     * name. That's the whole trade-off of this branch.
     * @param name_token the interned header name token.
     * @param value the header value.
     */
    HeaderField(types::Token name_token, std::string_view value)
        requires IsStatic
        : m_name{name_token}, m_value{value} {}

    /**
     * @brief Builds a dynamic header field with an arbitrary string name — only exists when
     * `IsStatic` is false, opposite trade-off from the `Token` ctor above.
     * @param name the header name. Can't be empty or this throws, no exceptions to that rule.
     * @param value the header value.
     * @throws std::runtime_error if `name` is empty — no nameless headers allowed, that's
     * straight cooked and gets rejected on sight.
     */
    HeaderField(std::string_view name, std::string_view value)
        requires(!IsStatic)
        : m_name{std::string(name)}, m_value{value} {
        if (name.empty()) {
            throw std::runtime_error("Empty name");
        }
    }

    // Accessors
    /**
     * @brief Grabs the header name, either the `Token` (static flavor) or `std::string`
     * (dynamic flavor) depending on `IsStatic` — whichever one this instantiation locked in.
     * @return the stored name, whatever type it happens to be for this instantiation.
     */
    [[nodiscard]] const auto &get_name() const noexcept { return m_name; }
    /**
     * @brief Grabs the header value, no funny business, straight from storage.
     * @return the stored value string.
     */
    [[nodiscard]] const std::string &get_value() const noexcept { return m_value; }

    // Logic for name size
    /**
     * @brief Estimates how many bytes this field costs to keep around — name cost plus value
     * size plus a flat overhead per entry (this is for cache/table sizing math, not the actual
     * wire size, don't confuse the two).
     * @return the estimated byte footprint of this field.
     */
    [[nodiscard]] std::size_t size() const noexcept {
        // Static fields carry a fixed-size Token for the name, dynamic fields carry the whole
        // string — name cost differs between the two branches, value + overhead stays the same.
        if constexpr (IsStatic) {
            return sizeof(types::Token) + m_value.size() + consts::ENTRY_OVERHEAD;
        } else {
            return m_name.size() + m_value.size() + consts::ENTRY_OVERHEAD;
        }
    }

    /**
     * @brief Checks if the value's blank, quick vibe check before you go using it.
     * @return true if the value is empty, false if there's something actually in there.
     */
    [[nodiscard]] bool is_empty() const noexcept { return m_value.empty(); }

    // Setters (Only for dynamic version)
    /**
     * @brief Overwrites the name — only exists for the dynamic (`!IsStatic`) flavor, since the
     * static one's name is locked in for good at construction via the `Token`, no swapping it
     * out later.
     * @param name the new name. Can't be empty or this throws, same rule as the ctor.
     * @throws std::runtime_error if `name` is empty.
     */
    void set_name(std::string name)
        requires(!IsStatic)
    {
        if (name.empty()) {
            throw std::runtime_error("Empty name");
        }
        m_name = std::move(name);
    }

    /**
     * @brief Overwrites the value. No validation here, empty's fine, go wild.
     * @param value the new value.
     */
    void set_value(std::string value) { m_value = std::move(value); }

    /**
     * @brief Checks two fields for equality — both name and value gotta match or it's a hard
     * no, no partial credit here.
     * @param other the field to compare against.
     * @return true if name and value are both equal, false otherwise.
     */
    bool operator==(const HeaderField &other) const noexcept {

        return m_name == other.m_name && m_value == other.m_value;
    };

  private:
    std::conditional_t<IsStatic, types::Token, std::string> m_name;
    std::string m_value;
};

// Export a common type for header entries, which can be either static or dynamic
using HeaderEntry =
    std::variant<std::shared_ptr<HeaderField<true>>, std::shared_ptr<HeaderField<false>>>;

} // namespace interfaces::io
