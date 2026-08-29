module;

export module yoshi_omah_lay_stable_hlo:operation;

import std;
import cc_abi_builder_generator;
import cc_abi_builder_intern;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import :dtype;
import :shape;
import :parameter_view;
import :schema_attribute_view;

export namespace cc::stable_hlo {

// One built op call — opcode, bound operands, bound attrs, bound results — OR an unbound schema
// entry describing an op KIND (placeholder operands/results, real attr metadata, no value).
// Structurally the same shape as a ice::builder::Definition (name/inputs/outputs/
// attrs) either way, so this implements that interface directly rather than being two separate
// StableHLO-only types (a bound Operation plus a schema-side Definition) with no relation to it. The
// compiled-in op schema table (schema/table.cppm) is a vector<Operation> of never-finalized-with-real-
// operands entries, built the same append_parameter/append_attr/append_result way a normal
// caller builds a real Operation — see build.cc — just with placeholder Shapes and no bound attr
// values, and never appended to a Function or rendered (check() only ever runs from inside
// render(), see there).
class Operation : public ice::builder::Definition
{
public:
    // `summary` defaults empty — every bound op has one (Operation::get_summary() always returned
    // empty before this class also covered the schema-entry case); only schema entries built by
    // build.cc's generated table supply a real one.
    Operation(std::string opcode, std::string category, std::string summary = {}) :
        m_opcode{std::move(opcode)},
        m_category{std::move(category)},
        m_summary{std::move(summary)}
    {
    }

    void append_parameter(const Parameter& operand)
    {
        m_parameters.push_back(operand);
    }

    // Takes an already-built Attribute (name/cpp_type/optional/list, plus whatever value the
    // caller attached via Attribute::append_value) and copies it in — same construct-then-
    // append concept as append_parameter/append_result: the caller builds the Attribute, this
    // only appends it, so const& (copies in).
    void append_attr(const Attribute& attribute)
    {
        m_attrs.push_back(attribute);
    }

    // Appends one already-computed result Parameter — shape inference (what a result's shape/
    // id should be) is the caller's job, done before this Operation is handed to Function::add_op.
    void append_result(const Parameter& result)
    {
        m_results.push_back(result);
    }

    // --- ice::builder::Definition ---
    ice::String get_name() const override
    {
        return ice::String{m_opcode};
    }

    ice::String get_summary() const override
    {
        return ice::String{m_summary};
    }

    ice::String get_description() const override
    {
        return ice::String{"category:" + m_category};
    }

    // Each returns a 1-D tensor of opaque Parameter*/Attribute* handles (list/array
    // carrier contract of TF_Tensor_Handle). The handles are heap copies with
    // parameter-slot context applied; ownership of each transfers to the C side,
    // which frees them with parameter__destroy/attribute__destroy.
    std::expected<ice::TensorHandle, ice::Status>
    get_inputs(ice::TensorHandle /*out*/) const override
    {
        int64_t count = static_cast<int64_t>(m_parameters.size());
        auto handle = make_handle_tensor(count);
        if (!handle) {
            return handle;
        }
        auto** dst = static_cast<void**>(m_tensor_runtime->get_data(handle->get_handle()));
        for (int64_t i = 0; i < count; ++i) {
            dst[i] = new Parameter(
                m_parameters[static_cast<size_t>(i)].with_context(static_cast<int>(i), true)
            );
        }
        return handle;
    }

    std::expected<ice::TensorHandle, ice::Status>
    get_outputs(ice::TensorHandle /*out*/) const override
    {
        int64_t count = static_cast<int64_t>(m_results.size());
        auto handle = make_handle_tensor(count);
        if (!handle) {
            return handle;
        }
        auto** dst = static_cast<void**>(m_tensor_runtime->get_data(handle->get_handle()));
        for (int64_t i = 0; i < count; ++i) {
            dst[i] = new Parameter(
                m_results[static_cast<size_t>(i)].with_context(static_cast<int>(i), false)
            );
        }
        return handle;
    }

    std::expected<ice::TensorHandle, ice::Status>
    get_attrs(ice::TensorHandle /*out*/) const override
    {
        int64_t count = static_cast<int64_t>(m_attrs.size());
        auto handle = make_handle_tensor(count);
        if (!handle) {
            return handle;
        }
        auto** dst = static_cast<void**>(m_tensor_runtime->get_data(handle->get_handle()));
        for (int64_t i = 0; i < count; ++i) {
            dst[i] = new Attribute(m_attrs[static_cast<size_t>(i)]);
        }
        return handle;
    }

    // --- StableHLO-specific ---

    const Parameter& get_operand(std::size_t index) const
    {
        return m_parameters.at(index);
    }

    const Parameter& get_result(std::size_t index) const
    {
        return m_results.at(index);
    }

    // A view over m_results — safe to hand out even across further add_op() calls on the
    // owning Function: m_results is this Operation's own independently-heap-allocated vector, so
    // moving the Operation itself (e.g. Function::m_ops reallocating) relocates the 3 pointer/size/
    // capacity words of m_results via its move constructor, not the Parameter array m_results
    // itself owns — the data this span points at doesn't move.
    [[nodiscard]] std::span<const Parameter> get_results() const
    {
        return m_results;
    }

    [[nodiscard]] std::expected<ice::String, ice::Status>
    render(int indent_level) const
    {
        auto checked = check();
        if (!checked) {
            return std::unexpected{checked.error()};
        }

        std::string result_ids;
        for (std::size_t i = 0; i < m_results.size(); ++i) {
            if (i > 0) {
                result_ids += ", ";
            }
            result_ids += m_results[i].get_id();
        }

        std::string prefix(static_cast<std::size_t>(indent_level) * 4, ' ');
        ice::StringHive hive;
        hive.append(ice::String{prefix});

        if (m_category == "unary") {
            hive.append(ice::String{std::format(
                "{} = stablehlo.{} {} : {}", result_ids, m_opcode, m_parameters[0].get_id(),
                m_parameters[0].get_shape()
            )});
        } else if (m_category == "binary") {
            hive.append(ice::String{std::format(
                "{} = stablehlo.{} {}, {} : {}", result_ids, m_opcode, m_parameters[0].get_id(),
                m_parameters[1].get_id(), m_parameters[0].get_shape()
            )});
        } else if (m_category == "comparison") {
            std::string suffix =
                m_attrs.empty() || !m_attrs[0].get_value() ? std::string{} : *m_attrs[0].get_value();
            hive.append(ice::String{std::format(
                "{} = stablehlo.compare {}, {}, {} : ({}, {}) -> {}", result_ids,
                m_parameters[0].get_id(), m_parameters[1].get_id(), suffix,
                m_parameters[0].get_shape(), m_parameters[1].get_shape(), m_results[0].get_shape()
            )});
        } else {
            std::string operand_ids;
            std::string operand_types;
            for (std::size_t i = 0; i < m_parameters.size(); ++i) {
                if (i > 0) {
                    operand_ids += ", ";
                    operand_types += ", ";
                }
                operand_ids += m_parameters[i].get_id();
                operand_types += std::format("{}", m_parameters[i].get_shape());
            }
            std::string attr_text;
            for (std::size_t i = 0; i < m_attrs.size(); ++i) {
                if (i > 0) {
                    attr_text += ", ";
                }
                attr_text += std::format(
                    "{} = {}", m_attrs[i].get_name().to_std_string(),
                    m_attrs[i].get_value().value_or(std::string{})
                );
            }
            std::string attr_block =
                attr_text.empty() ? std::string{} : std::format(" {{{}}}", attr_text);
            std::string result_types;
            for (std::size_t i = 0; i < m_results.size(); ++i) {
                if (i > 0) {
                    result_types += ", ";
                }
                result_types += std::format("{}", m_results[i].get_shape());
            }
            std::string result_type_text =
                m_results.size() == 1 ? result_types : std::format("({})", result_types);
            hive.append(ice::String{std::format(
                "{} = \"stablehlo.{}\"({}){} : ({}) -> {}", result_ids, m_opcode, operand_ids,
                attr_block, operand_types, result_type_text
            )});
        }

        hive.append_newline();
        return hive.get();
    }

private:
    // Pure completeness check — no computation, no mutation: does this Operation have the right number
    // of parameters/attrs/results for its own category? All the actual shape inference (a
    // result's shape/id, operand-shape compatibility) is the caller's job, done via
    // append_parameter()/append_attr()/append_result() before this Operation is ever appended anywhere
    // (see Function::add_op). Private — only render() calls it, at render time, not append time.
    [[nodiscard]] std::expected<void, ice::Status> check() const
    {
        if (m_category == "unary" && m_parameters.size() != 1) {
            return std::unexpected{ice::Status{std::format(
                "{}: unary op requires 1 operand, got {}", m_opcode, m_parameters.size()
            )}};
        }
        if (m_category == "binary" && m_parameters.size() != 2) {
            return std::unexpected{ice::Status{std::format(
                "{}: binary op requires 2 operands, got {}", m_opcode, m_parameters.size()
            )}};
        }
        if (m_category == "comparison") {
            if (m_parameters.size() != 2) {
                return std::unexpected{ice::Status{std::format(
                    "compare: requires 2 operands, got {}", m_parameters.size()
                )}};
            }
            bool has_direction = std::ranges::any_of(m_attrs, [](const Attribute& attr) {

                return attr.get_name().to_std_string() == "comparison_direction";

            });
            if (!has_direction) {
                return std::unexpected{ice::Status{"compare: comparison_direction attr required"}};
            }
        }
        if (m_results.empty()) {
            return std::unexpected{
                ice::Status{std::format("{}: at least one result is required", m_opcode)}
            };
        }
        return {};
    }

    std::string m_opcode;
    std::string m_category;
    std::string m_summary;
    std::vector<Parameter> m_parameters;
    std::vector<Attribute> m_attrs;
    std::vector<Parameter> m_results;
};

} // namespace cc::stable_hlo
