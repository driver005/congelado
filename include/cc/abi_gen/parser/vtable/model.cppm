export module cc_abi_gen_parser:vtable_model;

import std;
import :slot_slot;

export namespace cc_abi_gen::parser::vtable {

// The parsed shape of one include/c/extern/<domain>/<domain>.h vtable header.
class Model
{
public:
    Model(
        std::string && &struct_name,
        std::string&& struct_size_macro,
        std::string&& domain_name,
        std::string&& class_name,
        std::vector<Slot>&& slots
    ) :
        m_struct_name(std::move(struct_name)),
        m_struct_size_macro(std::move(struct_size_macro)),
        m_domain_name(std::move(domain_name)),
        m_class_name(std::move(class_name)),
        m_slots(std::move(slots))
    {
    }

    // Example: ice::String::create({})
    std::string wrape_type(std::string_view domain, std::string_view argument_name)
    {
        return std::format("{}::{}::wrap({})", domain, m_class_name, argument_name);
    }

    // Example: "{}.get_handle()",
    std::string unwrape_type(std::string_view argument_name)
    {
        return std::format("{}.get_handle()", argument_name);
    }

    // Example: const ice::String &
    std::string get_ponintee_type(std::string_view namespace_name)
    {
        return std::format("const {}::{} &", namespace_name, m_class_name);
    }

    // Example: TF_String
    std::string& get_struct_name() const
    {
        return m_struct_name;
    }

    // Example: TF_STRING_STRUCT_SIZE
    const std::string& get_struct_size_macro() const
    {
        return m_struct_size_macro;
    }

    // Example: string
    const std::string& get_domain_name() const
    {
        return m_domain_name;
    }

    // Example: String
    const std::string& get_class_name() const
    {
        return m_class_name;
    }

    const std::span<const Slot> get_slots() const
    {
        return std::span<const Slot>{m_slots};
    }

private:
    std::string m_struct_name;
    std::string m_struct_size_macro;
    std::string m_domain_name;
    std::string m_class_name;
    std::vector<Slot> m_slots;
};

} // namespace cc_abi_gen::parser::vtable
