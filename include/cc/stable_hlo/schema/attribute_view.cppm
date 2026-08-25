module;

export module cc_stable_hlo:schema_attribute_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :types;

export namespace cc::stable_hlo {

// Unbound attribute view — describes an attribute KIND from the schema, not a bound value.
// The schema-side sibling of op/attribute_view.cppm's StableHloOpAttributeView.
class StableHloAttributeView : public ice::GeneratorAttributeViewBase {
  public:
    explicit StableHloAttributeView(const StableHloAttrSchema &schema) : m_schema{schema} {}

    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_schema.name}; }
    ice::StringBuilder get_description() const override {
        return ice::StringBuilder{m_schema.optional ? "optional:true" : "optional:false"};
    }
    ice::StringBuilder get_full_type() const override { return ice::StringBuilder{m_schema.cpp_type}; }
    ice::StringBuilder get_base_type() const override { return ice::StringBuilder{m_schema.cpp_type}; }
    bool is_list() const override { return m_schema.list; }

  private:
    const StableHloAttrSchema &m_schema;
};

} // namespace cc::stable_hlo
