module;

export module cc_stable_hlo:definition_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :types;
import :parameter_view;
import :schema_attribute_view;

export namespace cc::stable_hlo {

// Unbound definition view — describes an op KIND from the schema table, sibling of
// op/op.cppm's StableHloOp (which implements the same interface for one bound call).
class StableHloDefinitionView : public ice::GeneratorDefinitionViewBase {
  public:
    explicit StableHloDefinitionView(const StableHloOpSchema &schema) : m_schema{schema} {}

    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_schema.name}; }
    ice::StringBuilder get_summary() const override { return ice::StringBuilder{m_schema.summary}; }
    ice::StringBuilder get_description() const override {

        return ice::StringBuilder{"category:" + m_schema.category};

    }
    std::size_t get_input_count() const override { return m_schema.inputs.size(); }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_input(std::size_t index) const override {

        if (index >= m_schema.inputs.size()) return nullptr;
        return std::make_unique<StableHloParameterView>(m_schema.inputs[index], static_cast<int>(index));

    }
    std::size_t get_output_count() const override {

        if (m_schema.output_count < 0) return 1;
        return m_schema.output_count == 0 ? 1 : static_cast<std::size_t>(m_schema.output_count);

    }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_output(std::size_t index) const override {

        std::size_t count = get_output_count();
        if (index >= count) return nullptr;
        std::string name = count == 1 ? "result" : ("result" + std::to_string(index));
        StableHloParamSchema result_param{std::move(name), m_schema.output_count < 0};
        return std::make_unique<StableHloParameterView>(std::move(result_param), static_cast<int>(index));

    }
    std::size_t get_attr_count() const override { return m_schema.attrs.size(); }
    std::unique_ptr<ice::GeneratorAttributeViewBase> get_attr(std::size_t index) const override {

        if (index >= m_schema.attrs.size()) return nullptr;
        return std::make_unique<StableHloAttributeView>(m_schema.attrs[index]);

    }

  private:
    const StableHloOpSchema &m_schema;
};

} // namespace cc::stable_hlo
