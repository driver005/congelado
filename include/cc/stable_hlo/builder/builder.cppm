module;

export module cc_stable_hlo:builder;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :dtype;
import :shape;
import :error;
import :value;
import :op;
import :function;
import :mlir_module;
import :types;
import :table;
import :definition_view;
import :source_code;

export namespace cc::stable_hlo {

// StableHloBuilder ties everything together: implements ice::GeneratorBuilderBase directly
// (it IS the generator, not a separate class beside one) and registers itself into
// cc_abi_builder_generator's GeneratorBuilderRegistry under "stablehlo" at static init (see
// builder/registrar.cppm). Nothing under cc/abi ever imports cc_stable_hlo — this is the one
// place this module touches cc/abi, and only downward.
class StableHloBuilder : public ice::GeneratorBuilderBase {
  public:
    explicit StableHloBuilder(std::string module_name = {}) : m_module{std::move(module_name)} {}

    // Registered factory (GeneratorBuilderRegistry, name "stablehlo"). source_dir goes
    // unused — the schema is compiled in, not read from a path at runtime; kept because the
    // Factory signature is fixed by cc_abi_builder_generator.
    static std::unique_ptr<ice::GeneratorBuilderBase> create(std::string_view output_dir, std::string_view) {
        auto builder = std::make_unique<StableHloBuilder>();
        builder->m_output_dir = output_dir;
        return builder;
    }

    // --- ice::GeneratorBuilderBase — the schema surface (which ops StableHLO supports) ---
    std::size_t get_definition_count() const override { return stable_hlo_op_schema_table().size(); }
    std::unique_ptr<ice::GeneratorDefinitionViewBase> get_definition(std::size_t index) const override {
        const auto &table = stable_hlo_op_schema_table();
        if (index >= table.size()) return nullptr;
        return std::make_unique<StableHloDefinitionView>(table[index]);
    }
    void write_file(std::string_view path, const ice::GeneratorSourceCodeBase &code) override {
        std::filesystem::path full_path = std::filesystem::path{m_output_dir} / path;
        std::error_code ec;
        std::filesystem::create_directories(full_path.parent_path(), ec);
        std::ofstream out{full_path};
        if (out) out << code.render().to_std_string();
    }
    void write_module(std::string_view path) override {
        // Generic "one func per op" catalog, built independently of this instance's own
        // m_module — exercises every schema entry with placeholder shapes/attrs via the same
        // add_op() path a normal caller uses, just driven by the schema table instead of
        // explicit call sites.
        StableHloModule catalog{"ops_catalog"};
        const StableHloShape default_shape{{4}, StableHloDType::i32()};

        for (const auto &schema : stable_hlo_op_schema_table()) {
            std::vector<StableHloValue> args;
            args.reserve(schema.inputs.size());
            for (std::size_t i = 0; i < schema.inputs.size(); ++i) {
                args.emplace_back("%arg" + std::to_string(i), default_shape);
            }
            StableHloFunction &function = catalog.add_function(schema.name, args);

            std::vector<std::pair<std::string, std::string>> attrs;
            attrs.reserve(schema.attrs.size());
            for (const auto &attr : schema.attrs) {
                attrs.emplace_back(attr.name, default_attr_text(attr.cpp_type));
            }

            std::size_t result_count = schema.output_count <= 0 ? 1 : static_cast<std::size_t>(schema.output_count);
            std::vector<StableHloShape> result_shapes(result_count, default_shape);

            auto op = StableHloOp::create_explicit(schema.name, args, std::move(attrs), std::move(result_shapes),
                                                    [&function] { return function.next_id(); });
            if (!op) continue; // best-effort catalog: skip anything that fails to build

            std::vector<StableHloValue> returns;
            returns.reserve(op->get_output_count());
            for (std::size_t i = 0; i < op->get_output_count(); ++i) {
                returns.push_back(op->get_result(i));
            }
            function.add_op(std::move(*op));
            function.set_returns(std::move(returns));
        }

        StableHloSourceCode sink;
        catalog.render_into(sink);

        std::filesystem::path full_path = std::filesystem::path{m_output_dir} / path;
        std::error_code ec;
        std::filesystem::create_directories(full_path.parent_path(), ec);
        std::ofstream out{full_path};
        if (out) out << sink.render().to_std_string();
    }
    void set_name(std::string_view name) override { m_name = name; }
    ice::StringBuilder get_name() const override { return ice::StringBuilder{m_name}; }

    // --- Tree construction — what a normal caller uses directly ---
    StableHloBuilder &add_function(std::string name, std::vector<StableHloShape> arg_shapes) {
        std::vector<StableHloValue> args;
        args.reserve(arg_shapes.size());
        for (std::size_t i = 0; i < arg_shapes.size(); ++i) {
            args.emplace_back("%arg" + std::to_string(i), std::move(arg_shapes[i]));
        }
        m_current_function = &m_module.add_function(std::move(name), std::move(args));
        return *this;
    }
    StableHloBuilder &add_main(std::vector<StableHloShape> arg_shapes) {
        return add_function("main", std::move(arg_shapes));
    }
    [[nodiscard]] const StableHloValue &get_argument(std::size_t index) const {
        return m_current_function->get_arguments().at(index);
    }

    [[nodiscard]] std::expected<std::vector<StableHloValue>, StableHloError>
    add_op(std::string_view opcode, std::span<const StableHloValue> operands,
          std::vector<std::pair<std::string, std::string>> attrs,
          std::vector<StableHloShape> result_shapes = {}) {
        if (!m_current_function) {
            return std::unexpected{StableHloError{"add_op called with no current function — call add_function first"}};
        }
        const StableHloOpSchema *schema = find_op_schema(opcode);
        std::string category = schema ? schema->category : std::string{"explicit"};

        std::expected<StableHloOp, StableHloError> op = std::unexpected{StableHloError{"unreachable"}};
        if (category == "unary" && operands.size() == 1) {
            op = StableHloOp::create_unary(std::string{opcode}, operands[0], m_current_function->next_id());
        } else if (category == "binary" && operands.size() == 2) {
            op = StableHloOp::create_binary(std::string{opcode}, operands[0], operands[1], m_current_function->next_id());
        } else if (category == "comparison" && operands.size() == 2) {
            std::string direction;
            std::optional<std::string> compare_type;
            for (const auto &[attr_name, attr_value] : attrs) {
                if (attr_name == "comparison_direction") direction = attr_value;
                else if (attr_name == "compare_type") compare_type = attr_value;
            }
            op = StableHloOp::create_comparison(operands[0], operands[1], direction, compare_type,
                                                m_current_function->next_id());
        } else {
            if (result_shapes.empty()) {
                return std::unexpected{StableHloError{std::format("{}: result_shapes required", opcode)}};
            }
            op = StableHloOp::create_explicit(std::string{opcode}, operands, std::move(attrs),
                                              std::move(result_shapes),
                                              [this] { return m_current_function->next_id(); });
        }
        if (!op) return std::unexpected{op.error()};

        std::vector<StableHloValue> results;
        results.reserve(op->get_output_count());
        for (std::size_t i = 0; i < op->get_output_count(); ++i) {
            results.push_back(op->get_result(i));
        }
        m_current_function->add_op(std::move(*op));
        return results;
    }

    std::expected<void, StableHloError> add_return(std::vector<StableHloValue> values) {
        if (!m_current_function) {
            return std::unexpected{StableHloError{"add_return called with no current function"}};
        }
        m_current_function->set_returns(std::move(values));
        return {};
    }

    [[nodiscard]] std::expected<std::string, StableHloError> build() const {
        if (m_module.empty()) {
            return std::unexpected{StableHloError{"StableHloBuilder has no functions to build"}};
        }
        StableHloSourceCode sink;
        m_module.render_into(sink);
        return sink.render().to_std_string();
    }

  private:
    static std::string default_attr_text(const std::string &cpp_type) {
        if (cpp_type == "std::vector<std::int64_t>") return "array<i64>";
        if (cpp_type == "std::vector<bool>") return "array<i1>";
        if (cpp_type == "std::string") return "";
        if (cpp_type == "bool") return "true";
        return "0";
    }

    static const StableHloOpSchema *find_op_schema(std::string_view opcode) {
        for (const auto &schema : stable_hlo_op_schema_table()) {
            if (schema.name == opcode) return &schema;
        }
        return nullptr;
    }

    std::string m_output_dir;
    std::string m_name;
    StableHloModule m_module;
    StableHloFunction *m_current_function{nullptr};
};

#ifdef CONGELADO_TEST
namespace tests {
using namespace boost::ut;

suite<"StableHloBuilder"> stable_hlo_builder_suite = [] {
    "build() fails with no functions added"_test = [] {
        StableHloBuilder builder{"empty_module"};
        auto result = builder.build();
        expect(not result.has_value());
    };

    "add_main + add_op(\"add\") renders a module with one function"_test = [] {
        StableHloBuilder builder{"my_module"};
        builder.add_main({StableHloShape::scalar(StableHloDType::f32())});
        auto operand = builder.get_argument(0);
        auto result = builder.add_op("add", std::vector<StableHloValue>{operand, operand}, {});
        expect(result.has_value());
        expect(builder.add_return(*result).has_value());

        auto built = builder.build();
        expect(built.has_value());
        expect(built->contains("stablehlo.add"));
        expect(built->contains("module @my_module {"));
    };
};

} // namespace tests
#endif

} // namespace cc::stable_hlo
