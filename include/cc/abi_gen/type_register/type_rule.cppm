export module cc_abi_gen_type_register:type_rule;

import std;

export namespace cc_abi_gen::type_register {

// How one raw-C pointee type (e.g. TF_TString, TF_Worker) crosses the raw/C++ boundary.
//
// Intern: a hand-written primitive under a fixed namespace (ice::String, ice::Status,
// ice::TensorHandle) — wrap/unwrap expressions are plain prefix+argument+suffix concatenation,
// not routed through TemplateRenderer, since a literal expression like "ice::TensorHandle{{{
// argument }}}" would collide with TemplateRenderer's own "{{ }}" delimiters.
//
// VtableModel: another domain's own generated wrapper class (namespace_name::ClassName), whose
// wrap/unwrap follows the one fixed "{{ namespace }}::{{ class }}::wrap(arg)" / "arg.get_handle()"
// pattern every generated class shares.
class TypeRule
{
public:
    enum class Kind
    {
        Intern,
        VtableModel
    };

    TypeRule() = default;

    static TypeRule make_intern(
        std::string class_name,
        std::string wrap_prefix,
        std::string wrap_suffix,
        std::string unwrap_prefix = "",
        std::string unwrap_suffix = ".get_handle()"
    ) noexcept
    {
        TypeRule rule;
        rule.m_kind = Kind::Intern;
        rule.m_class_name = std::move(class_name);
        rule.m_wrap_prefix = std::move(wrap_prefix);
        rule.m_wrap_suffix = std::move(wrap_suffix);
        rule.m_unwrap_prefix = std::move(unwrap_prefix);
        rule.m_unwrap_suffix = std::move(unwrap_suffix);
        return rule;
    }

    static TypeRule make_vtable_model(std::string class_name) noexcept
    {
        TypeRule rule;
        rule.m_kind = Kind::VtableModel;
        rule.m_class_name = std::move(class_name);
        rule.m_unwrap_prefix = "";
        rule.m_unwrap_suffix = ".get_handle()";
        return rule;
    }

    ~TypeRule() = default;
    TypeRule(const TypeRule&) = default;
    TypeRule& operator=(const TypeRule&) = default;
    TypeRule(TypeRule&&) = default;
    TypeRule& operator=(TypeRule&&) = default;

    // Example: "const ice::String &" / "const ice::builder::Worker &"
    std::string cpp_parameter_type(std::string_view namespace_name) const noexcept
    {
        return "const " + std::string{namespace_name} + "::" + m_class_name + " &";
    }

    // Example: ice::String::create(value_json) / ice::builder::Worker::wrap(worker)
    std::string raw_to_cpp(std::string_view namespace_name, std::string_view argument) const
        noexcept
    {
        if (m_kind == Kind::Intern) {
            return m_wrap_prefix + std::string{argument} + m_wrap_suffix;
        }

        return std::string{namespace_name} + "::" + m_class_name + "::wrap(" + std::string{argument}
             + ")";
    }

    // Example: value_json.get_handle()
    std::string cpp_to_raw(std::string_view argument) const noexcept
    {
        return m_unwrap_prefix + std::string{argument} + m_unwrap_suffix;
    }

    Kind get_kind() const noexcept
    {
        return m_kind;
    }

    // Example: String
    const std::string& get_class_name() const noexcept
    {
        return m_class_name;
    }

private:
    Kind m_kind{Kind::Intern};
    std::string m_class_name;
    std::string m_wrap_prefix;
    std::string m_wrap_suffix;
    std::string m_unwrap_prefix;
    std::string m_unwrap_suffix{".get_handle()"};
};

} // namespace cc_abi_gen::type_register
