module;

export module cc_stable_hlo:function;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :value;
import :value_parameter_view;
import :op;

export namespace cc::stable_hlo {

// One function body — also a GeneratorDefinitionViewBase: a named thing with bound inputs
// (its arguments) and bound outputs (its return values). No attrs (a correct answer, not a
// stub — functions carry none).
class StableHloFunction : public ice::GeneratorDefinitionViewBase {
  public:
    StableHloFunction(std::string name, std::vector<StableHloValue> arguments)
        : m_name{std::move(name)}, m_arguments{std::move(arguments)} {}

    StableHloFunction &add_op(StableHloOp op) {
        m_ops.push_back(std::move(op));
        return *this;
    }
    void set_returns(std::vector<StableHloValue> returns) { m_returns = std::move(returns); }
    [[nodiscard]] std::string next_id() { return "%" + std::to_string(m_next_value_id++); }

    // --- ice::GeneratorDefinitionViewBase ---
    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_name}; }
    ice::StringBuilder get_summary() const override { return ice::StringBuilder{}; }
    ice::StringBuilder get_description() const override { return ice::StringBuilder{}; }
    std::size_t get_input_count() const override { return m_arguments.size(); }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_input(std::size_t index) const override {
        if (index >= m_arguments.size()) return nullptr;
        return std::make_unique<StableHloValueParameterView>(m_arguments[index], static_cast<int>(index), true);
    }
    std::size_t get_output_count() const override { return m_returns.size(); }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_output(std::size_t index) const override {
        if (index >= m_returns.size()) return nullptr;
        return std::make_unique<StableHloValueParameterView>(m_returns[index], static_cast<int>(index), false);
    }
    std::size_t get_attr_count() const override { return 0; }
    std::unique_ptr<ice::GeneratorAttributeViewBase> get_attr(std::size_t) const override { return nullptr; }

    // --- StableHLO-specific ---
    std::size_t get_op_count() const { return m_ops.size(); }
    const StableHloOp &get_op(std::size_t index) const { return m_ops.at(index); }
    const std::vector<StableHloValue> &get_arguments() const { return m_arguments; }
    const std::vector<StableHloValue> &get_returns() const { return m_returns; }

    void render_into(ice::GeneratorSourceCodeBase &sink) const {
        std::string params;
        for (std::size_t i = 0; i < m_arguments.size(); ++i) {
            if (i > 0) params += ", ";
            params += std::format("{}: {}", m_arguments[i].get_id(), m_arguments[i].get_shape());
        }
        std::string result_types;
        for (std::size_t i = 0; i < m_returns.size(); ++i) {
            if (i > 0) result_types += ", ";
            result_types += std::format("{}", m_returns[i].get_shape());
        }
        sink.add_line(std::format("func.func @{}({}) -> ({}) {{", m_name, params, result_types));
        sink.increase_indent();
        for (const auto &op : m_ops) {
            op.render_into(sink);
        }
        std::string return_ids;
        for (std::size_t i = 0; i < m_returns.size(); ++i) {
            if (i > 0) return_ids += ", ";
            return_ids += m_returns[i].get_id();
        }
        sink.add_line(std::format("return {} : {}", return_ids, result_types));
        sink.decrease_indent();
        sink.add_line("}");
    }

  private:
    std::string m_name;
    std::vector<StableHloValue> m_arguments;
    std::vector<StableHloOp> m_ops;
    std::vector<StableHloValue> m_returns;
    std::size_t m_next_value_id{0};
};

} // namespace cc::stable_hlo
