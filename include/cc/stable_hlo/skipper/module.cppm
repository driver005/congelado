module;

export module cc_stable_hlo:module;

import std;
import cc_abi_builder_generator;
import cc_abi_sonic_intern;
import :function;
import :error;

export namespace cc::stable_hlo {

// The whole module — also a ice::builder::generator::Definition: a named thing with no inputs,
// outputs, or attrs of its own (all correct, empty answers — a module binds none of these
// itself). Owns its functions by value — a caller builds a whole Function standalone (arguments/
// ops/returns all populated) and hands it to append_function(), never holds a live Function&
// into this module's own storage across more than one call, so plain-vector reallocation is a
// non-issue (same build-then-append shape Function::add_op uses for Operation).
class Module : public ice::builder::generator::Definition
{
public:
    explicit Module(std::string name) :
        m_name{std::move(name)}
    {
    }

    // Appends an already-built Function (arguments/ops/returns all populated by the caller)
    // into this module's storage — copies it in (const& — the caller keeps ownership of their
    // own Function). Can't fail — Module has nothing to validate about a Function it didn't
    // build itself.
    void append_function(const Function& function)
    {
        m_functions.push_back(function);
    }

    // --- ice::builder::generator::Definition ---
    ice::sonic::StringRuntime get_name() const override
    {
        return ice::sonic::StringRuntime{m_name};
    }

    ice::sonic::StringRuntime get_summary() const override
    {
        return ice::sonic::StringRuntime{};
    }

    ice::sonic::StringRuntime get_description() const override
    {
        return ice::sonic::StringRuntime{};
    }

    std::size_t get_input_count() const override
    {
        return 0;
    }

    std::unique_ptr<ice::builder::generator::Parameter> get_input(std::size_t) const override
    {
        return nullptr;
    }

    std::size_t get_output_count() const override
    {
        return 0;
    }

    std::unique_ptr<ice::builder::generator::Parameter> get_output(std::size_t) const override
    {
        return nullptr;
    }

    std::size_t get_attr_count() const override
    {
        return 0;
    }

    std::unique_ptr<ice::builder::generator::Attribute> get_attr(std::size_t) const override
    {
        return nullptr;
    }

    // --- StableHLO-specific ---
    std::size_t get_function_count() const
    {
        return m_functions.size();
    }

    const Function& get_function(std::size_t index) const
    {
        return m_functions.at(index);
    }

    bool empty() const
    {
        return m_functions.empty();
    }

    [[nodiscard]] std::expected<ice::sonic::StringRuntime, Error>
    render(int indent_level = 0) const
    {
        auto checked = check();
        if (!checked) {
            return std::unexpected{checked.error()};
        }
        std::string prefix(static_cast<std::size_t>(indent_level) * 4, ' ');
        ice::sonic::StringHive hive;
        hive.append(ice::sonic::StringRuntime{prefix + std::format("module @{} {{", m_name)});
        hive.append_newline();
        for (const auto& function: m_functions) {
            auto rendered = function.render(indent_level + 1);
            if (!rendered) {
                return std::unexpected{rendered.error()};
            }
            hive.append(*rendered);
        }
        hive.append(ice::sonic::StringRuntime{prefix + "}"});
        hive.append_newline();
        return hive.get();
    }

private:
    // The one real invariant Module has: at least one function. Same condition empty() already
    // answers, phrased as the same check()/std::expected<void, Error> shape Operation::check() uses.
    // Private — only render() calls it, at render time, not append time.
    [[nodiscard]] std::expected<void, Error> check() const
    {
        if (m_functions.empty()) {
            return std::unexpected{Error{"Module has no functions"}};
        }
        return {};
    }

    std::string m_name;
    std::vector<Function> m_functions;
};

} // namespace cc::stable_hlo
