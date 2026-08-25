export module cc_stable_hlo:value;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import :shape;
import :dtype;

export namespace cc::stable_hlo {

// An SSA reference into a function body under construction — "%0", "%arg0", etc. — plus the
// shape it was produced with. Every op call returns one (or several, for multi-result ops)
// and consumes any earlier one, which is what makes the IR tree DAG-shaped rather than a
// linear chain: any prior StableHloValue can feed any later op call, not just the
// immediately preceding one.
class StableHloValue {
  public:
    StableHloValue(std::string id, StableHloShape shape) : m_id{std::move(id)}, m_shape{std::move(shape)} {}

    [[nodiscard]] const std::string &get_id() const noexcept { return m_id; }
    [[nodiscard]] const StableHloShape &get_shape() const noexcept { return m_shape; }

  private:
    std::string m_id;
    StableHloShape m_shape;
};

} // namespace cc::stable_hlo

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"StableHloValue"> stable_hlo_value_suite = [] {
    "get_id/get_shape return exactly what the ctor stored"_test = [] {
        StableHloValue value{"%0", StableHloShape::scalar(StableHloDType::f32())};
        expect(value.get_id() == "%0");
        expect(std::format("{}", value.get_shape()) == "tensor<f32>");
    };
};

} // namespace cc::stable_hlo::tests
#endif
