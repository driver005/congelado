module;

export module cc_stable_hlo:type_info_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :shape;
import :dtype;

export namespace cc::stable_hlo {

// Bound-instance type-info view: reflects a real StableHloShape's element type (from_shape),
// or — for the unbound schema side (schema/parameter_view.cppm) — only variadic-ness is known
// ahead of a real value (from_schema).
class StableHloTypeInfoView : public ice::GeneratorTypeInfoViewBase {
  public:
    static StableHloTypeInfoView from_shape(const StableHloShape &shape, bool is_read_only) {

        return StableHloTypeInfoView{static_cast<int>(shape.get_dtype().get_kind()), false, is_read_only};

    }
    static StableHloTypeInfoView from_schema(bool is_list) {

        return StableHloTypeInfoView{-1, is_list, true};

    }

    int get_data_type() const override { return m_data_type; }
    ice::StringBuilder get_type_attr_name() const override { return ice::StringBuilder{"T"}; }
    bool is_read_only() const override { return m_is_read_only; }
    bool is_list() const override { return m_is_list; }

  private:
    StableHloTypeInfoView(int data_type, bool is_list, bool is_read_only)
        : m_data_type{data_type}, m_is_list{is_list}, m_is_read_only{is_read_only} {}

    int m_data_type;
    bool m_is_list;
    bool m_is_read_only;
};

} // namespace cc::stable_hlo
