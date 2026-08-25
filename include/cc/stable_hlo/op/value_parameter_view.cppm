module;

export module cc_stable_hlo:value_parameter_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :value;
import :type_info_view;

export namespace cc::stable_hlo {

// Bound-instance parameter view: wraps one real StableHloValue (an operand, a result, a
// function argument, or a function return value). Holds an owned copy — some call sites
// (e.g. a definition's synthesized "result" parameter) have nothing longer-lived to
// reference, so every use here is by value rather than mixing reference and value storage.
class StableHloValueParameterView : public ice::GeneratorParameterViewBase {
  public:
    StableHloValueParameterView(StableHloValue value, int position, bool is_read_only)
        : m_value{std::move(value)}, m_position{position}, m_is_read_only{is_read_only} {}

    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_value.get_id()}; }
    ice::StringBuilder get_description() const override { return ice::StringBuilder{}; }
    int get_position() const override { return m_position; }
    std::unique_ptr<ice::GeneratorTypeInfoViewBase> get_type() const override {
        return std::make_unique<StableHloTypeInfoView>(StableHloTypeInfoView::from_shape(m_value.get_shape(), m_is_read_only));
    }

  private:
    StableHloValue m_value;
    int m_position;
    bool m_is_read_only;
};

} // namespace cc::stable_hlo
