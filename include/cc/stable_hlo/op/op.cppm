module;

export module cc_stable_hlo:op;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :dtype;
import :shape;
import :error;
import :value;
import :value_parameter_view;
import :op_attribute_view;

export namespace cc::stable_hlo {

// One built op call — opcode, bound operands, bound attrs, bound results. Structurally the
// same shape as a GeneratorDefinitionViewBase (name/inputs/outputs/attrs), just bound to
// real values instead of abstract parameter descriptions, so this implements that interface
// directly rather than being a StableHLO-only type with no relation to it.
class StableHloOp : public ice::GeneratorDefinitionViewBase {
  public:
    StableHloOp(std::string opcode, std::string category, std::vector<StableHloValue> operands,
               std::vector<std::pair<std::string, std::string>> attrs, std::vector<StableHloValue> results)
        : m_opcode{std::move(opcode)}, m_category{std::move(category)}, m_operands{std::move(operands)},
          m_attrs{std::move(attrs)}, m_results{std::move(results)} {}

    // --- Factories: this is where "shape inference" lives — validate operand shapes, compute
    // the result shape(s) per category, as part of constructing the StableHloOp itself. ---

    [[nodiscard]] static std::expected<StableHloOp, StableHloError>
    create_unary(std::string opcode, const StableHloValue &operand, std::string result_id) {
        StableHloValue result{std::move(result_id), operand.get_shape()};
        return StableHloOp{std::move(opcode), "unary", {operand}, {}, {std::move(result)}};
    }

    [[nodiscard]] static std::expected<StableHloOp, StableHloError>
    create_binary(std::string opcode, const StableHloValue &lhs, const StableHloValue &rhs, std::string result_id) {
        if (!(lhs.get_shape() == rhs.get_shape())) {
            return std::unexpected{StableHloError{std::format("{}: operand shape mismatch ({} vs {})", opcode,
                                                               lhs.get_shape(), rhs.get_shape())}};
        }
        StableHloValue result{std::move(result_id), lhs.get_shape()};
        return StableHloOp{std::move(opcode), "binary", {lhs, rhs}, {}, {std::move(result)}};
    }

    [[nodiscard]] static std::expected<StableHloOp, StableHloError>
    create_comparison(const StableHloValue &lhs, const StableHloValue &rhs, std::string comparison_direction,
                      std::optional<std::string> compare_type, std::string result_id) {
        if (lhs.get_shape().get_dims() != rhs.get_shape().get_dims()) {
            return std::unexpected{StableHloError{
                std::format("compare: operand shape mismatch ({} vs {})", lhs.get_shape(), rhs.get_shape())}};
        }
        StableHloShape result_shape{lhs.get_shape().get_dims(), StableHloDType::i1()};
        std::string direction_attr = compare_type.has_value()
                                         ? std::format("{}, {}", comparison_direction, *compare_type)
                                         : comparison_direction;
        std::vector<std::pair<std::string, std::string>> attrs{{"comparison_direction", std::move(direction_attr)}};
        StableHloValue result{std::move(result_id), std::move(result_shape)};
        return StableHloOp{"compare", "comparison", {lhs, rhs}, std::move(attrs), {std::move(result)}};
    }

    [[nodiscard]] static std::expected<StableHloOp, StableHloError>
    create_explicit(std::string opcode, std::span<const StableHloValue> operands,
                    std::vector<std::pair<std::string, std::string>> attrs,
                    std::vector<StableHloShape> result_shapes, const std::function<std::string()> &next_id) {
        if (result_shapes.empty()) {
            return std::unexpected{StableHloError{std::format("{}: at least one result shape is required", opcode)}};
        }
        std::vector<StableHloValue> results;
        results.reserve(result_shapes.size());
        for (auto &shape : result_shapes) {
            results.emplace_back(next_id(), std::move(shape));
        }
        return StableHloOp{std::move(opcode), "explicit",
                           std::vector<StableHloValue>{operands.begin(), operands.end()}, std::move(attrs),
                           std::move(results)};
    }

    // --- ice::GeneratorDefinitionViewBase ---
    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_opcode}; }
    ice::StringBuilder get_summary() const override { return ice::StringBuilder{}; }
    ice::StringBuilder get_description() const override { return ice::StringBuilder{"category:" + m_category}; }
    std::size_t get_input_count() const override { return m_operands.size(); }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_input(std::size_t index) const override {
        if (index >= m_operands.size()) return nullptr;
        return std::make_unique<StableHloValueParameterView>(m_operands[index], static_cast<int>(index), true);
    }
    std::size_t get_output_count() const override { return m_results.size(); }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_output(std::size_t index) const override {
        if (index >= m_results.size()) return nullptr;
        return std::make_unique<StableHloValueParameterView>(m_results[index], static_cast<int>(index), false);
    }
    std::size_t get_attr_count() const override { return m_attrs.size(); }
    std::unique_ptr<ice::GeneratorAttributeViewBase> get_attr(std::size_t index) const override {
        if (index >= m_attrs.size()) return nullptr;
        return std::make_unique<StableHloOpAttributeView>(m_attrs[index]);
    }

    // --- StableHLO-specific ---
    const StableHloValue &get_operand(std::size_t index) const { return m_operands.at(index); }
    const StableHloValue &get_result(std::size_t index) const { return m_results.at(index); }

    void render_into(ice::GeneratorSourceCodeBase &sink) const {
        std::string result_ids;
        for (std::size_t i = 0; i < m_results.size(); ++i) {
            if (i > 0) result_ids += ", ";
            result_ids += m_results[i].get_id();
        }

        if (m_category == "unary") {
            sink.add_line(std::format("{} = stablehlo.{} {} : {}", result_ids, m_opcode, m_operands[0].get_id(),
                                      m_operands[0].get_shape()));
            return;
        }
        if (m_category == "binary") {
            sink.add_line(std::format("{} = stablehlo.{} {}, {} : {}", result_ids, m_opcode, m_operands[0].get_id(),
                                      m_operands[1].get_id(), m_operands[0].get_shape()));
            return;
        }
        if (m_category == "comparison") {
            std::string suffix = m_attrs.empty() ? std::string{} : m_attrs[0].second;
            sink.add_line(std::format("{} = stablehlo.compare {}, {}, {} : ({}, {}) -> {}", result_ids,
                                      m_operands[0].get_id(), m_operands[1].get_id(), suffix,
                                      m_operands[0].get_shape(), m_operands[1].get_shape(), m_results[0].get_shape()));
            return;
        }
        std::string operand_ids;
        std::string operand_types;
        for (std::size_t i = 0; i < m_operands.size(); ++i) {
            if (i > 0) {
                operand_ids += ", ";
                operand_types += ", ";
            }
            operand_ids += m_operands[i].get_id();
            operand_types += std::format("{}", m_operands[i].get_shape());
        }
        std::string attr_text;
        for (std::size_t i = 0; i < m_attrs.size(); ++i) {
            if (i > 0) attr_text += ", ";
            attr_text += std::format("{} = {}", m_attrs[i].first, m_attrs[i].second);
        }
        std::string attr_block = attr_text.empty() ? std::string{} : std::format(" {{{}}}", attr_text);
        std::string result_types;
        for (std::size_t i = 0; i < m_results.size(); ++i) {
            if (i > 0) result_types += ", ";
            result_types += std::format("{}", m_results[i].get_shape());
        }
        std::string result_type_text = m_results.size() == 1 ? result_types : std::format("({})", result_types);
        sink.add_line(std::format("{} = \"stablehlo.{}\"({}){} : ({}) -> {}", result_ids, m_opcode, operand_ids,
                                  attr_block, operand_types, result_type_text));
    }

  private:
    std::string m_opcode;
    std::string m_category;
    std::vector<StableHloValue> m_operands;
    std::vector<std::pair<std::string, std::string>> m_attrs;
    std::vector<StableHloValue> m_results;
};

} // namespace cc::stable_hlo
