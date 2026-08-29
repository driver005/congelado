module;

#include <cstddef>
#include <cstdint>

export module yoshi_omah_lay_stable_hlo:builder;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :dtype;
import :shape;
import :parameter_view;
import :operation;
import :function;
import :hlo_module;

export namespace cc::stable_hlo {

// Builder ties everything together: implements ice::builder::Generator directly (it IS
// the generator, not a separate class beside one). Nothing under cc/abi ever imports
// yoshi_omah_lay_stable_hlo — this is the one place this module touches cc/abi, and only downward.
// Only implements the interface's typed-API tier (own native construction methods — append_module
// here, add_parameter/add_op on Function itself) — the interface's second, optional,
// C-ABI-crossable generic construction tier (enter_border_patrol) isn't opted into, so it
// keeps the base class's default "not supported" answer. stable_hlo.cc registers a
// TF_InitGenerator-style factory that constructs this class into
// ice::sonic::RegistrationRuntime under type="generator", name="stablehlo", so
// ice::sonic::Generator can resolve it in-process without a direct cc_abi ->
// yoshi_omah_lay_stable_hlo dependency.
class Builder : public ice::builder::Generator
{
public:
    Builder() = default;

    // Tensor runtime the definitions' get_inputs/outputs/attrs allocate through.
    explicit Builder(ice::sonic::Tensor& tensor_runtime) :
        ice::builder::Generator{tensor_runtime}
    {
    }

    // --- ice::builder::Generator ---

    // Module implements ice::builder::Definition too, so the modules this Builder
    // actually holds double as its definition list — no separate static schema catalog.
    // Each definition is handed out as a heap copy (stable across later append_module
    // calls, which can reallocate m_modules); ownership of the copy transfers to the C
    // side, which frees it with definition__destroy. The tensor data is an array of
    // opaque Definition* handles (the list/array-carrier contract of TF_Tensor_Handle).
    std::expected<ice::TensorHandle, ice::Status>
    get_definitions() const noexcept override
    {
        if (!m_tensor_runtime) {
            return std::unexpected{ice::Status{"Builder has no tensor runtime"}};
        }
        int64_t count = static_cast<int64_t>(m_modules.size());
        size_t bytes = static_cast<size_t>(count) * sizeof(void*);
        auto res = m_tensor_runtime->allocate_tensor(
            ice::DataTypeEnum::Uint8,
            std::span{&count, 1},
            bytes
        );
        if (!res) {
            return std::unexpected{res.error()};
        }
        auto* raw = res.value();
        ice::TensorHandle handle{raw};
        auto** dst = static_cast<void**>(m_tensor_runtime->get_data(raw));
        for (int64_t i = 0; i < count; ++i) {
            auto* module = new Module(m_modules[static_cast<size_t>(i)]);
            if (m_tensor_runtime) {
                module->set_tensor_runtime(*m_tensor_runtime);
            }
            dst[i] = module;
        }
        return handle;
    }

    void set_name(std::string_view name) noexcept override
    {
        m_name = name;
    }

    ice::String get_name() const noexcept override
    {
        return ice::String{m_name};
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

    [[nodiscard]] std::expected<ice::String, ice::Status> build() const noexcept override
    {
        if (m_modules.empty()) {
            return std::unexpected{ice::Status{"Builder has no modules"}};
        }
        ice::StringHive hive;
        for (const auto& module: m_modules) {
            auto rendered = module.render();
            if (!rendered) {
                return std::unexpected{rendered.error()};
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

    suite<"Builder"> stable_hlo_builder_suite = []
    {
        "build() fails with no modules appended"_test = []
        {
            Builder builder;
            auto result = builder.build();
            expect(not result.has_value());
        };

        "build a Function with add_op(\"add\"), append it into a Module, append_module it "
        "renders one function"_test = []
        {
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
