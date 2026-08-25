module;

export module cc_stable_hlo:op_attribute_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;

export namespace cc::stable_hlo {

// One bound attribute (name, already-rendered textual value) on a built StableHloOp.
class StableHloOpAttributeView : public ice::GeneratorAttributeViewBase {
  public:
    explicit StableHloOpAttributeView(std::pair<std::string, std::string> attr) : m_attr{std::move(attr)} {}

    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_attr.first}; }
    ice::StringBuilder get_description() const override { return ice::StringBuilder{}; }
    ice::StringBuilder get_full_type() const override { return ice::StringBuilder{"std::string"}; }
    ice::StringBuilder get_base_type() const override { return ice::StringBuilder{"std::string"}; }
    bool is_list() const override { return false; }

  private:
    std::pair<std::string, std::string> m_attr;
};

} // namespace cc::stable_hlo
