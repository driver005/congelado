export module cc_abi_gen_parser:model;

import std;
import :slot_slot;
import :field;

export namespace cc_abi_gen::parser {

enum class Kind : std::uint8_t
{
    VTABLE,
    STRUCT,
    ENUM,
};

class Model
{
public:
    // Ops vtable struct: include/c/extern/<domain>/<domain>.h
    Model(
        std::string&& struct_name,
        std::string&& struct_size_macro,
        std::string&& domain_name,
        std::string&& class_name,
        std::string&& header_path,
        std::vector<slot::Slot>&& slots
    ) :
        m_kind{Kind::VTABLE},
        m_c_name{std::move(struct_name)},
        m_class_name{std::move(class_name)},
        m_domain_name{std::move(domain_name)},
        m_header_path{std::move(header_path)},
        m_struct_size_macro{std::move(struct_size_macro)},
        m_slots{std::move(slots)}
    {
    }

    // Plain struct or enum, not part of a vtable flow.
    Model(
        Kind kind,
        std::string&& c_name,
        std::string&& class_name,
        std::string&& domain_name,
        std::string&& header_path
    ) :
        m_kind{kind},
        m_c_name{std::move(c_name)},
        m_class_name{std::move(class_name)},
        m_domain_name{std::move(domain_name)},
        m_header_path{std::move(header_path)}
    {
    }

    ~Model() = default;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    Model& add_field(Field&& field) noexcept
    {
        m_fields.push_back(std::move(field));
        return *this;
    }

    Model& add_enumerator(std::string&& enumerator) noexcept
    {
        m_enumerators.push_back(std::move(enumerator));
        return *this;
    }

    Model& add_struct_size(bool has_struct_size) noexcept
    {
        m_struct_size = has_struct_size;
        return *this;
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const slot::Slot>>
    find_slot(std::string_view slot_name) const noexcept
    {
        auto found = std::ranges::find_if(
            m_slots,
            [slot_name](const slot::Slot& slot)
            {
                return slot.get_name() == slot_name;
            }
        );
        if (found == m_slots.end()) {
            return std::nullopt;
        }

        return std::cref(*found);
    }

    // Example: ice::sonic::String::wrap({})
    [[nodiscard]] std::string
    wrape_type(std::string_view domain, std::string_view argument_name) const noexcept
    {
        return std::format(
            "self.wrap(std::type_identity<{}::sonic::{}>{{}}, {})",
            domain,
            m_class_name,
            argument_name
        );
    }

    // Example: "{}.get_handle()",
    [[nodiscard]] std::string unwrape_type(std::string_view argument_name) const noexcept
    {
        return std::format("{}.get_handle()", argument_name);
    }

    // Example: ice::sonic::Status
    [[nodiscard]] std::string to_sonic_type(std::string_view namespace_name) const noexcept
    {
        return std::format("{}::sonic::{}", namespace_name, m_class_name);
    }

    // Example: const ice::sonic::String &
    [[nodiscard]] std::string to_pointee_type(std::string_view namespace_name) const noexcept
    {
        return std::format("const {} &", to_sonic_type(namespace_name));
    }

    // Returns the entire tier path (including sub-tiers), stopping before the domain.
    std::expected<std::string, std::string> to_tier() const
    {
        if (m_kind != Kind::VTABLE) {
            return std::filesystem::path{m_header_path}
                .parent_path()
                .parent_path()
                .generic_string();
        }

        auto header_reverse = m_header_path | std::views::reverse;

        auto last_slash_it = std::ranges::find(header_reverse, '/');
        if (last_slash_it == header_reverse.end()) {
            return std::string{};
        }

        auto second_last_slash_it =
            std::ranges::find(std::ranges::next(last_slash_it), header_reverse.end(), '/');

        if (second_last_slash_it == header_reverse.end()) {
            std::size_t idx = m_header_path.size() - 1 -
                              std::ranges::distance(header_reverse.begin(), last_slash_it);
            return m_header_path.substr(0, idx);
        }

        std::size_t idx = m_header_path.size() - 1 -
                          std::ranges::distance(header_reverse.begin(), second_last_slash_it);
        return m_header_path.substr(0, idx);
    }

    // Output partition name. vtable: header stem; value: snake_case of the class name.
    std::expected<std::string, std::string> to_file_name() const
    {
        if (m_kind != Kind::VTABLE) {
            std::string name;
            for (const char CHARACTER: m_class_name) {
                if ((std::isupper(static_cast<unsigned char>(CHARACTER)) != 0) && !name.empty()) {
                    name += '_';
                }
                name += static_cast<char>(std::tolower(static_cast<unsigned char>(CHARACTER)));
            }

            return name;
        }

        if (m_header_path.empty()) {
            return std::unexpected(std::string{"Header path is empty"});
        }
        std::string name = std::filesystem::path(m_header_path).stem().generic_string();
        constexpr std::array<std::string_view, 3> MODULE_KEYWORDS = {"module", "import", "export"};
        if (std::ranges::find(MODULE_KEYWORDS, name) != MODULE_KEYWORDS.end()) {
            name += '_';
        }

        return name;
    }

    // shared

    [[nodiscard]] const Kind& get_kind() const noexcept
    {
        return m_kind;
    }

    // Example: TF_StringOps (vtable) / TF_PoolId (value)
    [[nodiscard]] const std::string& get_c_name() const noexcept
    {
        return m_c_name;
    }

    // Example: String / PoolId
    [[nodiscard]] const std::string& get_class_name() const noexcept
    {
        return m_class_name;
    }

    // Example: string / stream_executor
    [[nodiscard]] const std::string& get_domain_name() const noexcept
    {
        return m_domain_name;
    }

    // Absolute path to the C header this struct was parsed from
    [[nodiscard]] const std::string& get_header_path() const noexcept
    {
        return m_header_path;
    }

    // vtable

    // Example: TF_STRING_STRUCT_SIZE
    [[nodiscard]] const std::string& get_struct_size_macro() const noexcept
    {
        return m_struct_size_macro;
    }

    [[nodiscard]] std::span<const slot::Slot> get_slots() const noexcept
    {
        return m_slots;
    }

    // value

    [[nodiscard]] bool has_struct_size() const noexcept
    {
        return m_struct_size;
    }

    [[nodiscard]] std::span<const Field> get_fields() const noexcept
    {
        return m_fields;
    }

    // C enumerator names in declaration order.
    [[nodiscard]] std::span<const std::string> get_enumerators() const noexcept
    {
        return m_enumerators;
    }

private:
    Kind m_kind;
    std::string m_c_name;
    std::string m_class_name;
    std::string m_domain_name;
    std::string m_header_path;
    std::string m_struct_size_macro;
    std::vector<slot::Slot> m_slots;
    bool m_struct_size{false};
    std::vector<Field> m_fields;
    std::vector<std::string> m_enumerators;
};

} // namespace cc_abi_gen::parser
