module;

export module cc_stable_hlo:schema_attribute_view;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;

export namespace cc::stable_hlo {

// Attribute — describes an attribute KIND (name + real cpp_type/optional/list metadata) from
// the schema, AND (optionally) the bound value for that attr on a real Operation — value is nullopt
// for a schema entry's attr (no bound value exists yet), set via append_value() for a real
// bound Operation's attr. One class either way, no separate parallel-vector-of-values on Operation (the old
// op/attribute.cppm — OpAttribute, a raw (name, value) pair with no type metadata — is gone,
// folded into this same class).
class Attribute : public ice::builder::generator::Attribute
{
public:
    Attribute(std::string name, std::string cpp_type, bool optional, bool list) :
        m_name{std::move(name)},
        m_cpp_type{std::move(cpp_type)},
        m_optional{optional},
        m_list{list}
    {
    }

    // Sets the bound value for this attr — construct-then-append, same shape as Operation's own
    // append_parameter/append_attr/append_result. Caller constructs the value, this only sets
    // it, so const& (copies in).
    void append_value(const std::optional<std::string>& value)
    {
        m_value = value;
    }

    [[nodiscard]] const std::optional<std::string>& get_value() const
    {
        return m_value;
    }

    ice::sonic::StringRuntime get_name() const override
    {
        return ice::sonic::StringRuntime{m_name};
    }

    ice::sonic::StringRuntime get_description() const override
    {

        return ice::sonic::StringRuntime{
            std::string{m_optional ? "optional:true" : "optional:false"}
        };
    }

    ice::sonic::StringRuntime get_full_type() const override
    {
        return ice::sonic::StringRuntime{m_cpp_type};
    }

    ice::sonic::StringRuntime get_base_type() const override
    {
        return ice::sonic::StringRuntime{m_cpp_type};
    }

    bool is_list() const override
    {
        return m_list;
    }

private:
    std::string m_name;
    std::string m_cpp_type;
    bool m_optional;
    bool m_list;
    std::optional<std::string> m_value;
};

} // namespace cc::stable_hlo
