module;

export module yoshi_omah_lay_stable_hlo:parameter_view;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_builder_generator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import :shape;
import :dtype;

export namespace cc::stable_hlo {

// A parameter — bound (a real SSA reference: "%0", "%arg0", ... plus the Shape it was produced
// with) or unbound (a schema entry describing an input/output KIND, not a bound value). One
// class either way: this used to be two (ValueParameter here, a separate schema-side Parameter
// with a bare `bool variadic` instead of a real Shape) — the schema side never had real shape
// data, so it filled the gap with a placeholder Shape whose only meaningful property is its
// rank (rank > 0 standing in for "variadic"), same as is_list() already reads off any real
// bound Shape. Everywhere the bound value's real Shape is read (Operation::render,
// Function::render format real MLIR types from it) keeps working unchanged; nothing reads
// a schema entry's placeholder Shape for anything but rank.
//
// position/is_read_only default to context-free values (-1/false, same convention as DType's
// context fields) since most of this type's life is spent as a plain DAG reference (an op's
// operand/result list, a function's argument/return list) — with_context() is used wherever one
// is actually being handed out as a real parameter (see Operation::get_input/get_output,
// Function::get_input/get_output, Operation's own compiled-table entries).
class Parameter : public ice::builder::Parameter
{
public:
    // position defaults so 2-arg construction ("next id + shape, no position yet" — every
    // freshly-produced op result) still reads naturally.
    Parameter(std::string name, Shape shape, int position = -1) :
        m_name{std::move(name)},
        m_shape{std::move(shape)},
        m_position{position}
    {
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const Shape& get_shape() const noexcept
    {
        return m_shape;
    }

    // Returns a copy carrying the given parameter-slot context — used wherever this parameter
    // is being handed out as an actual operand/result/argument/return/schema-input (as opposed
    // to just living in a plain list).
    [[nodiscard]] Parameter with_context(int position, bool is_read_only) const
    {
        Parameter copy = *this;
        copy.m_position = position;
        copy.m_is_read_only = is_read_only;
        return copy;
    }

    // --- ice::builder::Parameter ---
    ice::String get_name() const noexcept override
    {
        return ice::String{m_name};
    }

    ice::String get_description() const noexcept override
    {
        return ice::String{};
    }

    int get_position() const noexcept override
    {
        return m_position;
    }

    std::unique_ptr<ice::builder::TypeInfo> get_type() const noexcept override
    {
        return std::make_unique<DType>(
            m_shape.get_dtype().with_context(m_is_read_only, m_shape.get_rank() > 0)
        );
    }

private:
    std::string m_name;
    Shape m_shape;
    int m_position;
    bool m_is_read_only{false};
};

} // namespace cc::stable_hlo

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"Parameter"> stable_hlo_parameter_suite = []
{
    "get_id/get_shape return exactly what the ctor stored"_test = []
    {
        Parameter value{"%0", Shape::scalar(DType::f32())};
        expect(value.get_id() == "%0");
        expect(std::format("{}", value.get_shape()) == "tensor<f32>");
    };
};

} // namespace cc::stable_hlo::tests
#endif
