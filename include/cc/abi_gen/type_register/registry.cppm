export module cc_abi_gen_type_register:registry;

import std;
import cc_abi_gen_parser;
import :type_rule;

export namespace cc_abi_gen::type_register {

// Replaces and absorbs the old parser::Registry entirely: one struct-name -> vtable::Model map
// (built from a full up-front scan of every domain header, so a type used by one domain but
// defined in another stays visible regardless of parse order), plus a pointee-name -> TypeRule
// map used by the emitters to classify slot parameters.
class TypeRegister
{
public:
    using Iterator = std::unordered_map<std::string, parser::vtable::Model>::iterator;
    using ConstIterator = std::unordered_map<std::string, parser::vtable::Model>::const_iterator;

    TypeRegister() noexcept
    {
        seed_intern_types();
    }

    ~TypeRegister() = default;
    TypeRegister(const TypeRegister&) = delete;
    TypeRegister& operator=(const TypeRegister&) = delete;
    TypeRegister(TypeRegister&&) = default;
    TypeRegister& operator=(TypeRegister&&) = default;

    // Absorbs every vtable::Model discovered while parsing one header (or a whole domain scan),
    // registering each one both by struct name (for cli_runner's generate/check loop) and as a
    // VtableModel TypeRule (for cross-domain slot parameters referencing another domain's class).
    void absorb(std::vector<parser::vtable::Model>&& models) noexcept
    {
        for (parser::vtable::Model& model: models) {
            std::string struct_name = model.get_struct_name();

            m_type_rules.insert_or_assign(
                struct_name,
                TypeRule::make_vtable_model(model.get_class_name())
            );

            m_models.insert_or_assign(std::move(struct_name), std::move(model));
        }
    }

    std::optional<std::reference_wrapper<const parser::vtable::Model>>
    find_model(const std::string& struct_name) const noexcept
    {
        auto it = m_models.find(struct_name);
        if (it != m_models.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const TypeRule>>
    find_type_rule(std::string_view pointee_name) const noexcept
    {
        auto it = m_type_rules.find(std::string{pointee_name});
        if (it != m_type_rules.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    const std::unordered_map<std::string, parser::vtable::Model>& get_models() const noexcept
    {
        return m_models;
    }

    Iterator begin() noexcept
    {
        return m_models.begin();
    }

    Iterator end() noexcept
    {
        return m_models.end();
    }

    ConstIterator begin() const noexcept
    {
        return m_models.begin();
    }

    ConstIterator end() const noexcept
    {
        return m_models.end();
    }

private:
    // Hand-authored C++ wrapper idioms for the intern value types slots reference — these aren't
    // structurally derivable from the C headers alone (they're primitives under include/cc/abi/
    // primitives/, not generated vtable classes), so they're seeded manually. Verified directly
    // against include/cc/abi/primitives/{status,string,tensor_handle}.cppm.
    void seed_intern_types() noexcept
    {
        m_type_rules.insert_or_assign(
            "TF_Status",
            TypeRule::make_intern("Status", "ice::Status::create(", ")")
        );

        // TF_String and TF_TString alias the same underlying union — both wrap to ice::String.
        m_type_rules.insert_or_assign(
            "TF_TString",
            TypeRule::make_intern("String", "ice::String::create(", ")")
        );
        m_type_rules.insert_or_assign(
            "TF_String",
            TypeRule::make_intern("String", "ice::String::create(", ")")
        );

        m_type_rules.insert_or_assign(
            "TF_Tensor_Handle",
            TypeRule::make_intern("TensorHandle", "ice::TensorHandle{", "}")
        );

        // FilesystemOption::create takes a reference, not a pointer, unlike the other
        // seeded intern types — hence the explicit deref in the wrap prefix.
        m_type_rules.insert_or_assign(
            "TF_Filesystem_Option",
            TypeRule::make_intern("FilesystemOption", "ice::FilesystemOption::create(*", ")")
        );
    }

    std::unordered_map<std::string, parser::vtable::Model> m_models;
    std::unordered_map<std::string, TypeRule> m_type_rules;
};

} // namespace cc_abi_gen::type_register
