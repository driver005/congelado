module;

export module cc_stable_hlo:parameter_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :types;
import :type_info_view;

export namespace cc::stable_hlo {

// Unbound parameter view — describes an input/output KIND from the schema, not a bound value.
// The schema-side sibling of op/value_parameter_view.cppm's StableHloValueParameterView.
class StableHloParameterView : public ice::GeneratorParameterViewBase {
  public:
    StableHloParameterView(StableHloParamSchema schema, int position)
        : m_schema{std::move(schema)}, m_position{position} {}

    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_schema.name}; }
    ice::StringBuilder get_description() const override { return ice::StringBuilder{}; }
    int get_position() const override { return m_position; }
    std::unique_ptr<ice::GeneratorTypeInfoViewBase> get_type() const override {

        return std::make_unique<StableHloTypeInfoView>(
            StableHloTypeInfoView::from_schema(m_schema.variadic));

    }

  private:
    StableHloParamSchema m_schema;
    int m_position;
};

} // namespace cc::stable_hlo
