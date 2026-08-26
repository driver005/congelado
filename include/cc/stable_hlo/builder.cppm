module;

export module cc_stable_hlo:builder;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;
import :dtype;
import :shape;
import :parameter_view;
import :operation;
import :function;
import :module;

export namespace cc::stable_hlo {

// Builder ties everything together: implements ice::builder::generator::Builder directly (it IS
// the generator, not a separate class beside one). Nothing under cc/abi ever imports
// cc_stable_hlo — this is the one place this module touches cc/abi, and only downward. Only
// implements the interface's typed-API tier (own native construction methods — append_module
// here, add_parameter/add_op on Function itself) — the interface's second, optional,
// C-ABI-crossable generic construction tier isn't opted into: "stablehlo" is the only generator
// implementation that exists in this codebase, so there's nothing that actually needs to cross
// the C ABI or resolve through ice::builder::generator::BuilderRegistry today.
class Builder : public ice::builder::generator::Builder
{
public:
    Builder() = default;

    // --- ice::builder::generator::Builder ---

    // Module implements ice::builder::generator::Definition too, so the modules this Builder
    // actually holds double as its definition list — no separate static schema catalog.
    std::size_t get_definition_count() const override
    {
        return m_modules.size();
    }

    std::unique_ptr<ice::builder::generator::Definition>
    get_definition(std::size_t index) const override
    {
        if (index >= m_modules.size()) {
            return nullptr;
        }
        return std::make_unique<Module>(m_modules[index]);
    }

    void set_name(std::string_view name) override
    {
        m_name = name;
    }

    ice::sonic::StringRuntime get_name() const override
    {
        return ice::sonic::StringRuntime{m_name};
    }

    // --- Tree construction — what a normal caller uses directly ---

    // Appends an already-built Module (functions/ops all populated by the caller) into this
    // Builder's storage — copies it in (const&). Can't fail — Builder has nothing to validate
    // about a Module it didn't build itself (Module validates its own functions at render()
    // time).
    void append_module(const Module& module)
    {
        m_modules.push_back(module);
    }

    [[nodiscard]] std::expected<ice::sonic::StringRuntime, ice::sonic::StringRuntime>
    build() const override
    {
        if (m_modules.empty()) {
            return std::unexpected{ice::sonic::StringRuntime{std::string{"Builder has no modules"}}};
        }
        ice::sonic::StringHive hive;
        for (const auto& module: m_modules) {
            auto rendered = module.render();
            if (!rendered) {
                return std::unexpected{ice::sonic::StringRuntime{rendered.error().get_message()}};
            }
            hive.append(*rendered);
        }
        return hive.get();
    }

private:
    std::string m_name;
    std::vector<Module> m_modules;
};

#ifdef CONGELADO_TEST
namespace tests {
    using namespace boost::ut;

    suite<"Builder"> stable_hlo_builder_suite = [] {
        "build() fails with no modules appended"_test = [] {
            Builder builder;
            auto result = builder.build();
            expect(not result.has_value());
        };

        "build a Function with add_op(\"add\"), append it into a Module, append_module it "
        "renders one function"_test = [] {
            Builder builder;
            Function function{"main"};
            function.add_parameter(Shape::scalar(DType::f32()));
            Parameter operand = function.get_arguments().back();

            Operation op{"add", "binary"};
            op.append_parameter(operand);
            op.append_parameter(operand);
            op.append_result(Parameter{function.next_id(), operand.get_shape()});
            function.add_op(op);

            auto results = function.get_op(function.get_op_count() - 1).get_results();
            function.set_returns({results.begin(), results.end()});

            Module module{"my_module"};
            module.append_function(function);
            builder.append_module(module);

            auto built = builder.build();
            expect(built.has_value());
            auto built_text = built->to_std_string();
            expect(built_text.contains("stablehlo.add"));
            expect(built_text.contains("module @my_module {"));
        };
    };

} // namespace tests
#endif

} // namespace cc::stable_hlo
