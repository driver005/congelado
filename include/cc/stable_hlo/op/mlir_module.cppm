module;

export module cc_stable_hlo:mlir_module;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :function;

export namespace cc::stable_hlo {

// The whole module — also a GeneratorDefinitionViewBase: a named thing with no inputs,
// outputs, or attrs of its own (all correct, empty answers — a module binds none of these
// itself). Owns its functions via unique_ptr so references returned by add_function()/
// get_function() stay valid across further add_function() calls (a plain
// vector<StableHloFunction> would invalidate them on reallocation). Named "mlir_module", not
// "module", to avoid any ambiguity with C++'s own module keyword/concept.
class StableHloModule : public ice::GeneratorDefinitionViewBase {
  public:
    explicit StableHloModule(std::string name) : m_name{std::move(name)} {}

    StableHloFunction &add_function(std::string name, std::vector<StableHloValue> arguments) {

        m_functions.push_back(std::make_unique<StableHloFunction>(std::move(name), std::move(arguments)));
        return *m_functions.back();

    }

    // --- ice::GeneratorDefinitionViewBase ---
    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_name}; }
    ice::StringBuilder get_summary() const override { return ice::StringBuilder{}; }
    ice::StringBuilder get_description() const override { return ice::StringBuilder{}; }
    std::size_t get_input_count() const override { return 0; }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_input(std::size_t) const override { return nullptr; }
    std::size_t get_output_count() const override { return 0; }
    std::unique_ptr<ice::GeneratorParameterViewBase> get_output(std::size_t) const override { return nullptr; }
    std::size_t get_attr_count() const override { return 0; }
    std::unique_ptr<ice::GeneratorAttributeViewBase> get_attr(std::size_t) const override { return nullptr; }

    // --- StableHLO-specific ---
    std::size_t get_function_count() const { return m_functions.size(); }
    const StableHloFunction &get_function(std::size_t index) const { return *m_functions.at(index); }
    bool empty() const { return m_functions.empty(); }

    void render_into(ice::GeneratorSourceCodeBase &sink) const {

        sink.add_line(std::format("module @{} {{", m_name));
        sink.increase_indent();
        for (const auto &function : m_functions) {
            function->render_into(sink);
        }
        sink.decrease_indent();
        sink.add_line("}");
    }

  private:
    std::string m_name;
    std::vector<std::unique_ptr<StableHloFunction>> m_functions;

};

} // namespace cc::stable_hlo
