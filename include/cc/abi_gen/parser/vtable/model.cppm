module;
#include <expected>
#include <string>
export module cc_abi_gen_parser:vtable_model;

import std;
import :slot_slot;

export namespace cc_abi_gen::parser::vtable {

// The parsed shape of one include/c/extern/<domain>/<domain>.h vtable header.
class Model
{
public:
    Model(
        std::string&& struct_name,
        std::string&& struct_size_macro,
        std::string&& domain_name,
        std::string&& class_name,
        std::string&& header_path,
        std::vector<slot::Slot>&& slots
    ) :
        m_struct_name(std::move(struct_name)),
        m_struct_size_macro(std::move(struct_size_macro)),
        m_domain_name(std::move(domain_name)),
        m_class_name(std::move(class_name)),
        m_header_path(std::move(header_path)),
        m_slots(std::move(slots))
    {
    }

    ~Model() = default;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    Model& add_struct_name(std::string&& struct_name) noexcept
    {
        m_struct_name = std::move(struct_name);
        return *this;
    }

    Model& add_struct_size_macro(std::string&& struct_size_macro) noexcept
    {
        m_struct_size_macro = std::move(struct_size_macro);
        return *this;
    }

    Model& add_domain_name(std::string&& domain_name) noexcept
    {
        m_domain_name = std::move(domain_name);
        return *this;
    }

    Model& add_class_name(std::string&& class_name) noexcept
    {
        m_class_name = std::move(class_name);
        return *this;
    }

    Model& add_header_path(std::string&& header_path) noexcept
    {
        m_header_path = std::move(header_path);
        return *this;
    }

    Model& add_slot(slot::Slot&& slot) noexcept
    {
        m_slots.emplace_back(std::move(slot));
        return *this;
    }

    // Example: ice::sonic::String::wrap({})
    std::string wrape_type(std::string_view domain, std::string_view argument_name) const noexcept
    {
        return std::format("{}::sonic::{}::wrap({})", domain, m_class_name, argument_name);
    }

    // Example: "{}.get_handle()",
    std::string unwrape_type(std::string_view argument_name) const noexcept
    {
        return std::format("{}.get_handle()", argument_name);
    }

    // Example: const ice::sonic::String &
    std::string to_pointee_type(std::string_view namespace_name) const noexcept
    {
        return std::format("const {}::sonic::{} &", namespace_name, m_class_name);
    }

    // Returns the entire tier path (including sub-tiers), stopping before the domain.
    std::expected<std::string, std::string> to_tier() const
    {
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
            return std::string{m_header_path.substr(0, idx)};
        }

        std::size_t idx = m_header_path.size() - 1 -
                          std::ranges::distance(header_reverse.begin(), second_last_slash_it);
        return std::string{m_header_path.substr(0, idx)};
    }

    std::expected<std::string, std::string> to_file_name() const
    {
        if (m_header_path.empty()) {
            return std::unexpected(std::string{"Header path is empty"});
        }
        return std::filesystem::path(m_header_path).stem().generic_string();
    }

    void set_struct_name(std::string&& struct_name) noexcept
    {
        m_struct_name = std::move(struct_name);
    }

    void set_struct_size_macro(std::string&& struct_size_macro) noexcept
    {
        m_struct_size_macro = std::move(struct_size_macro);
    }

    void set_domain_name(std::string&& domain_name) noexcept
    {
        m_domain_name = std::move(domain_name);
    }

    void set_class_name(std::string&& class_name) noexcept
    {
        m_class_name = std::move(class_name);
    }

    void set_header_path(std::string&& header_path) noexcept
    {
        m_header_path = std::move(header_path);
    }

    void append_slot(slot::Slot&& slot) noexcept
    {
        m_slots.push_back(std::move(slot));
    }

    // Example: TF_String
    const std::string& get_struct_name() const noexcept
    {
        return m_struct_name;
    }

    // Example: TF_STRING_STRUCT_SIZE
    const std::string& get_struct_size_macro() const noexcept
    {
        return m_struct_size_macro;
    }

    // Example: string
    const std::string& get_domain_name() const noexcept
    {
        return m_domain_name;
    }

    // Example: String
    const std::string& get_class_name() const noexcept
    {
        return m_class_name;
    }

    // Absolute path to the C header this struct was parsed from
    const std::string& get_header_path() const noexcept
    {
        return m_header_path;
    }

    std::span<const slot::Slot> get_slots() const noexcept
    {
        return m_slots;
    }

private:
    std::string m_struct_name;
    std::string m_struct_size_macro;
    std::string m_domain_name;
    std::string m_class_name;
    std::string m_header_path;
    std::vector<slot::Slot> m_slots;
};

} // namespace cc_abi_gen::parser::vtable
