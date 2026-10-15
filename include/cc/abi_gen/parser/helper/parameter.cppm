module;

#include <cctype>

export module cc_abi_gen_parser:helper_parameter;

import std;

export namespace cc_abi_gen::parser::helper {

class Parameter
{
public:
    Parameter() = default;

    Parameter(std::string&& type, std::string&& pointee_name, std::string&& name) :
        m_type(std::move(type)),
        m_pointee_name(std::move(pointee_name)),
        m_name(std::move(name))
    {
    }

    ~Parameter() = default;
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) = default;
    Parameter& operator=(Parameter&&) = default;

    Parameter& add_type(std::string&& type) noexcept
    {
        m_type = std::move(type);
        return *this;
    }

    Parameter& add_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
        return *this;
    }

    Parameter& add_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
        return *this;
    }

    void set_type(std::string&& type) noexcept
    {
        m_type = std::move(type);
    }

    void set_pointee_name(std::string&& pointee_name) noexcept
    {
        m_pointee_name = std::move(pointee_name);
    }

    Parameter& add_scalar_pointer(bool scalar_pointer) noexcept
    {
        m_scalar_pointer = scalar_pointer;
        return *this;
    }

    void set_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
    }

    // Example: TF_TString
    std::string_view get_type() const noexcept
    {
        return m_type;
    }

    // Same as get_type() but with the record pointee spelled from the global namespace, so a
    // generated class sharing the handle's name (ice::sonic::TF_Tensor) cannot shadow it.
    // Example: "const TF_Status *" -> "const ::TF_Status *"
    std::string get_qualified_type() const
    {
        if (m_pointee_name.empty() ||
            !std::isupper(static_cast<unsigned char>(m_pointee_name.front()))) {
            return m_type;
        }

        const auto position = m_type.find(m_pointee_name);
        if (position == std::string::npos || (position > 0 && m_type[position - 1] == ':')) {
            return m_type;
        }

        std::string qualified = m_type;
        qualified.insert(position, "::");

        return qualified;
    }

    // Example: TF_Status_Handle
    std::string_view get_pointee_name() const noexcept
    {
        return m_pointee_name;
    }

    // True for a pointer to a struct/record type (e.g. TF_Status, TF_Buffer); false for raw scalar
    // pointers (e.g. int64_t*) and by-value params, which have no pointee at all. Handle types no
    // longer carry a distinguishing suffix — whether a given pointee actually names a registered
    // domain (as opposed to a plain value struct like TF_Job_Options) is decided by looking it up
    // in the registry, not by this check alone.
    bool has_pointee() const noexcept
    {
        return !m_pointee_name.empty();
    }

    // Registry lookup key for this parameter's pointee. The registry is keyed by the Ops struct's
    // own tag (e.g. "TF_StatusOps"), but a parameter referencing that domain names its handle
    // instead (e.g. "TF_Status* status") — appending "Ops" bridges the two.
    std::string get_registry_key() const noexcept
    {
        return m_pointee_name + "Ops";
    }

    // Example: argument_name
    std::string_view get_name() const noexcept
    {
        return m_name;
    }

    // True for `out_*` pointers to a non-const arithmetic or enum value (not char, not a record,
    // not a pointer to pointer). Whether it is really a single value and not an array is decided
    // against the whole slot by the emitter.
    bool is_scalar_out() const noexcept
    {
        return m_scalar_pointer && m_name.starts_with("out_");
    }

    // C++ spelling of the pointee for the reference form of a scalar out parameter.
    // Example: "int32_t" -> "std::int32_t", "_Bool" -> "bool", "TFDataTypeEnum" ->
    // "::TFDataTypeEnum"
    std::string get_value_type() const
    {
        static constexpr std::array<std::string_view, 12> k_std_names{
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
            "ptrdiff_t"
        };

        if (m_pointee_name == "_Bool") {
            return "bool";
        }
        if (std::ranges::find(k_std_names, std::string_view{m_pointee_name}) != k_std_names.end()) {
            return "std::" + m_pointee_name;
        }
        if (!m_pointee_name.empty() &&
            std::isupper(static_cast<unsigned char>(m_pointee_name.front()))) {
            return "::" + m_pointee_name;
        }

        return m_pointee_name;
    }

private:
    std::string m_type;
    std::string m_pointee_name;
    std::string m_name;
    bool m_scalar_pointer{false};
};

} // namespace cc_abi_gen::parser::helper
