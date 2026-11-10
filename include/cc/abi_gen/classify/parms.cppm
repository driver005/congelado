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
        std::optional<std::string> defaultValue = std::nullopt
    ) :
        m_type(std::move(type)),
        m_name(std::move(name)),
        m_default_value(std::move(defaultValue))
    {
    }

    ~Param() = default;

    Param(const Param& other) = default;
    Param& operator=(const Param& other) = default;
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

    Param& add_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
        return *this;
    }

    Param& add_scalar_pointer(bool scalar_pointer) noexcept
    {
        m_scalar_pointer = scalar_pointer;
        return *this;
    }

    Param& add_element_pointer(bool element_pointer) noexcept
    {
        m_element_pointer = element_pointer;
        return *this;
    }

    [[nodiscard]] bool is_pointer() const noexcept
    {
        return m_type.ends_with('*');
    }

    [[nodiscard]] bool is_const_pointee() const noexcept
    {
        return m_type.starts_with("const ");
    }

    // Registry name of a plain C struct or enum: "const TF_PoolId *" -> "TF_PoolId"
    [[nodiscard]] std::string get_value_key() const
    {
        if (is_pointer()) {
            return m_pointee_name;
        }

        std::string_view key = m_type;
        for (const std::string_view PREFIX: {"const ", "enum ", "struct "}) {
            if (key.starts_with(PREFIX)) {
                key.remove_prefix(PREFIX.size());
            }
        }

        return std::string{key};
    }

    // get_type() with the record pointee in the global namespace: "const ::TF_Status *"
    [[nodiscard]] std::string get_qualified_type() const
    {
        if (m_pointee_name.empty() ||
            std::isupper(static_cast<unsigned char>(m_pointee_name.front())) == 0) {
            return m_type;
        }

        const auto POSITION = m_type.find(m_pointee_name);
        if (POSITION == std::string::npos || (POSITION > 0 && m_type.at(POSITION - 1) == ':')) {
            return m_type;
        }

        std::string qualified = m_type;
        qualified.insert(POSITION, "::");

        return qualified;
    }

    // Example: TF_Status_Handle
    [[nodiscard]] std::string_view get_pointee_name() const noexcept
    {
        return m_pointee_name;
    }

    // True for a pointer to a struct/record type; false for scalar pointers and by-value params.
    [[nodiscard]] bool has_pointee() const noexcept
    {
        return !m_pointee_name.empty();
    }

    // Registry key of the pointee: the handle name plus "Ops" (TF_Status -> TF_StatusOps).
    [[nodiscard]] std::string get_registry_key() const noexcept
    {
        return m_pointee_name + "Ops";
    }

    // True for `out_*` pointers to a non-const number or enum, unless the slot is an array.
    [[nodiscard]] bool is_scalar_out() const noexcept
    {
        return m_scalar_pointer && m_name.starts_with("out_");
    }

    // True for a pointer to numbers or enums (not char, not bool): an array start with a length.
    [[nodiscard]] bool is_element_pointer() const noexcept
    {
        return m_element_pointer;
    }

    // True for a plain integer parameter that can carry an element count.
    [[nodiscard]] bool is_integer() const noexcept
    {
        static constexpr std::array<std::string_view, 8> K_INTEGERS{
            "int",
            "unsigned int",
            "size_t",
            "int32_t",
            "int64_t",
            "uint32_t",
            "uint64_t",
            "long",
        };

        return std::ranges::find(K_INTEGERS, std::string_view{m_type}) != K_INTEGERS.end();
    }

    // C++ spelling of the pointee: "int32_t" -> "std::int32_t", "_Bool" -> "bool"
    [[nodiscard]] std::string get_value_type() const
    {
        static constexpr std::array<std::string_view, 12> K_STD_NAMES{
            "int8_t",
            "int16_t",
            "int32_t",
            "int64_t",
            "uint8_t",
            "uint16_t",
            "uint32_t",
            "uint64_t",
            "intptr_t",
            "uintptr_t",
            "size_t",
            "ptrdiff_t",
        };

        if (m_pointee_name == "_Bool") {
            return "bool";
        }
        if (std::ranges::find(K_STD_NAMES, std::string_view{m_pointee_name}) != K_STD_NAMES.end()) {
            return "std::" + m_pointee_name;
        }
        if (!m_pointee_name.empty() &&
            std::isupper(static_cast<unsigned char>(m_pointee_name.front())) != 0) {
            return "::" + m_pointee_name;
        }

        return m_pointee_name;
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

        // Unnamed parameter, e.g. a deleted copy constructor `X(const X&)`.
        if (m_name.empty()) {
            return m_type;
        }

        if (includeDefault && m_default_value.has_value()) {
            return std::format("{} {} = {}", m_type, m_name, *m_default_value);
        }
        return std::format("{} {}", m_type, m_name);
    }

private:
    std::string m_type;
    std::string m_pointee_name;
    std::string m_name;
    std::optional<std::string> m_default_value;
    bool m_scalar_pointer{false};
    bool m_element_pointer{false};
};

} // namespace cc_abi_gen::classify
