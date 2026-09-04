export module cc_abi_gen_generator:slot_classifier;

import std;
import cc_abi_gen_parser;
import :known_type;
import :type_registry;

export namespace cc_abi_gen {

// Slot classification shared by BuilderEmitter and SonicEmitter — one ruleset applied
// identically on both sides of the generated C++ vtable wrapper. Every type substitution or
// conversion (beyond the fixed destroy/get_name/plugin_context/TF_Status structure) goes
// through the shared TypeRegistry, so a type this generator doesn't yet know how to wrap is
// flagged rather than silently passed through wrong.
class SlotClassifier
{
public:
    explicit SlotClassifier(TypeRegistry &type_registry) noexcept : m_type_registry(type_registry)
    {
    }

    bool is_destroy(const VtableSlot &slot)
    {
        return slot.m_name == "destroy";
    }

    bool is_get_name(const VtableSlot &slot)
    {
        return slot.m_name == "get_name";
    }

    bool is_fallible(const VtableSlot &slot)
    {
        return !slot.m_parameters.empty() && pointee_name(slot.m_parameters.back().m_type) == "TF_Status";
    }

    // Every slot's parameters minus the leading plugin_context and (if fallible) the trailing
    // TF_Status* — exactly what the C++-facing method signature exposes. A non-owning view over
    // slot's own storage (a contiguous sub-range of it) rather than a copy, so there's no vector
    // to own on either side.
    std::span<const Parameter> middle_parameters(const VtableSlot &slot)
    {
        std::size_t start = slot.m_parameters.empty() ? 0 : 1;
        std::size_t end = slot.m_parameters.size();
        if (is_fallible(slot) && end > start) {

            --end;
        }

        return std::span<const Parameter>(slot.m_parameters).subspan(start, end - start);
    }

    // C++-facing parameter type for one middle parameter. A registered type substitutes its
    // known C++ type; a non-pointer type (a callback typedef, an enum, a builtin) passes through
    // unchanged, since it isn't a wrapped value at all; any other pointer type not yet
    // registered is queued in the registry as a real gap instead of guessed at.
    std::string cpp_parameter_type(const Parameter &parameter)
    {
        std::string name = pointee_name(parameter.m_type);
        if (name.empty()) {

            return parameter.m_type;
        }

        if (const KnownType *known = m_type_registry.find(name)) {

            return known->m_cpp_parameter_type;
        }

        m_type_registry.note_unknown(name);

        return parameter.m_type;
    }

    // Turns a C argument name into its C++ value at the builder-lambda boundary (e.g.
    // "ice::String::create(key)"), per the registered type's wrap format — or the argument name
    // unchanged if the type isn't registered (already flagged by cpp_parameter_type) or is
    // registered with no wrap step of its own.
    std::string wrap_argument(const std::string &argument_name, const std::string &type)
    {
        std::string name = pointee_name(type);
        const KnownType *known = name.empty() ? nullptr : m_type_registry.find(name);
        if (known == nullptr || known->m_wrap_format.empty()) {

            return argument_name;
        }

        return std::vformat(known->m_wrap_format, std::make_format_args(argument_name));
    }

    // Turns a C++ value name back into its C-ABI argument at the sonic-call boundary (e.g.
    // "message.get_handle()"), per the registered type's unwrap format — or the value name
    // unchanged otherwise.
    std::string unwrap_argument(const std::string &argument_name, const std::string &type)
    {
        std::string name = pointee_name(type);
        const KnownType *known = name.empty() ? nullptr : m_type_registry.find(name);
        if (known == nullptr || known->m_unwrap_format.empty()) {

            return argument_name;
        }

        return std::vformat(known->m_unwrap_format, std::make_format_args(argument_name));
    }

private:
    std::string strip_spaces(const std::string &value)
    {
        std::string result;
        for (char character : value) {

            if (character != ' ') {

                result += character;
            }
        }

        return result;
    }

    // "const TF_TString *" -> "TF_TString"; "void*" and any non-pointer spelling (a callback
    // typedef, an enum, a builtin) -> "" (not a candidate for wrapping at all).
    std::string pointee_name(const std::string &type)
    {
        std::string stripped = strip_spaces(type);
        if (stripped == "void*" || stripped.empty() || stripped.back() != '*') {

            return "";
        }

        stripped.pop_back();
        if (stripped.starts_with("const")) {

            stripped.erase(0, 5);
        }

        return stripped;
    }

    TypeRegistry &m_type_registry;
};

} // namespace cc_abi_gen
