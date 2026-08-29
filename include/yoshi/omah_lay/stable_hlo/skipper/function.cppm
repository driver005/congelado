module;

export module yoshi_omah_lay_stable_hlo:function;

import std;
import cc_abi_builder_generator;
import cc_abi_builder_intern;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import :shape;
import :parameter_view;
import :operation;

export namespace cc::stable_hlo {

// One function body — also a ice::builder::Definition: a named thing with bound inputs
// (its arguments) and bound outputs (its return values). No attrs (a correct answer, not a
// stub — functions carry none).
class Function : public ice::builder::Definition
{
public:
    explicit Function(std::string name) :
        m_name{std::move(name)}
    {
    }

    // Synthesizes "%argN" from the current argument count, appends a bound Parameter.
    void append_arg(const Shape& shape)
    {
        m_arguments.emplace_back("%arg" + std::to_string(m_arguments.size()), shape);
    }

    // Same as append_arg — a caller who needs the just-added Parameter reads it back via
    // get_arguments().back() (copy it out by value if another add_parameter()/append_arg()
    // call happens before it's used — a further append can reallocate m_arguments).
    void add_parameter(const Shape& shape)
    {
        append_arg(shape);
    }

    // Pushes an already-produced result value straight onto the return list — no id synthesis
    // needed, it already has one (it came from a real op result).
    void append_return(const Parameter& value)
    {
        m_returns.push_back(value);
    }

    // Bulk-replace, for callers that already have a complete return list in hand (e.g. built
    // from get_op(get_op_count() - 1).get_results() after add_op()) — as opposed to
    // append_return's one-at-a-time use while a list is still being produced.
    void set_returns(const std::vector<Parameter>& returns)
    {
        m_returns = returns;
    }

    [[nodiscard]] std::string next_id()
    {
        return "%" + std::to_string(m_next_value_id++);
    }

    // Copies an already-built Operation into this function's own storage (const& — the caller
    // keeps ownership of their own op). Unconditional — Operation::check() is private now and
    // only ever runs from inside Operation::render(); nothing validates an Operation before it's
    // stored, only when something later tries to render it.
    void add_op(const Operation& op)
    {
        m_ops.push_back(op);
    }

    // --- ice::builder::Definition ---
    ice::String get_name() const override
    {
        return ice::String{m_name};
    }

    ice::String get_summary() const override
    {
        return ice::String{};
    }

    ice::String get_description() const override
    {
        return ice::String{};
    }

    // Tensor of opaque Parameter* handles — arguments (inputs) and returns (outputs)
    // as heap copies with parameter-slot context; attrs are always empty (functions
    // carry none). Ownership of each handle transfers to the C side.
    std::expected<ice::TensorHandle, ice::Status>
    get_inputs(ice::TensorHandle /*out*/) const override
    {
        int64_t count = static_cast<int64_t>(m_arguments.size());
        auto handle = make_handle_tensor(count);
        if (!handle) {
            return handle;
        }
        auto** dst = static_cast<void**>(m_tensor_runtime->get_data(handle->get_handle()));
        for (int64_t i = 0; i < count; ++i) {
            dst[i] = new Parameter(
                m_arguments[static_cast<size_t>(i)].with_context(static_cast<int>(i), true)
            );
        }
        return handle;
    }

    std::expected<ice::TensorHandle, ice::Status>
    get_outputs(ice::TensorHandle /*out*/) const override
    {
        int64_t count = static_cast<int64_t>(m_returns.size());
        auto handle = make_handle_tensor(count);
        if (!handle) {
            return handle;
        }
        auto** dst = static_cast<void**>(m_tensor_runtime->get_data(handle->get_handle()));
        for (int64_t i = 0; i < count; ++i) {
            dst[i] = new Parameter(
                m_returns[static_cast<size_t>(i)].with_context(static_cast<int>(i), false)
            );
        }
        return handle;
    }

    std::expected<ice::TensorHandle, ice::Status>
    get_attrs(ice::TensorHandle /*out*/) const override
    {
        return make_handle_tensor(0);
    }

    // --- StableHLO-specific ---
    std::size_t get_op_count() const
    {
        return m_ops.size();
    }

    const Operation& get_op(std::size_t index) const
    {
        return m_ops.at(index);
    }

    const std::vector<Parameter>& get_arguments() const
    {
        return m_arguments;
    }

    const std::vector<Parameter>& get_returns() const
    {
        return m_returns;
    }

    [[nodiscard]] std::expected<ice::String, ice::Status>
    render(int indent_level = 0) const
    {

        std::string params;
        for (std::size_t i = 0; i < m_arguments.size(); ++i) {
            if (i > 0) {
                params += ", ";
            }
            params += std::format("{}: {}", m_arguments[i].get_id(), m_arguments[i].get_shape());
        }
        std::string result_types;
        for (std::size_t i = 0; i < m_returns.size(); ++i) {
            if (i > 0) {
                result_types += ", ";
            }
            result_types += std::format("{}", m_returns[i].get_shape());
        }

        std::string prefix(static_cast<std::size_t>(indent_level) * 4, ' ');
        ice::StringHive hive;
        hive.append(ice::String{
            prefix + std::format("func.func @{}({}) -> ({}) {{", m_name, params, result_types)
        });
        hive.append_newline();
        for (const auto& op: m_ops) {
            auto rendered = op.render(indent_level + 1);
            if (!rendered) {
                return std::unexpected{rendered.error()};
            }
            hive.append(*rendered);
        }
        std::string return_ids;
        for (std::size_t i = 0; i < m_returns.size(); ++i) {
            if (i > 0) {
                return_ids += ", ";
            }
            return_ids += m_returns[i].get_id();
        }
        std::string inner_prefix(static_cast<std::size_t>(indent_level + 1) * 4, ' ');
        hive.append(ice::String{
            inner_prefix + std::format("return {} : {}", return_ids, result_types)
        });
        hive.append_newline();
        hive.append(ice::String{prefix + "}"});
        hive.append_newline();
        return hive.get();
    }

private:
    std::string m_name;
    std::vector<Parameter> m_arguments;
    std::vector<Operation> m_ops;
    std::vector<Parameter> m_returns;
    std::size_t m_next_value_id{0};
};

} // namespace cc::stable_hlo
