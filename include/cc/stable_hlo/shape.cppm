export module cc_stable_hlo:shape;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import :dtype;

export namespace cc::stable_hlo {

// A tensor shape: a dimension list plus its element type, rendered as StableHLO's
// "tensor<...>" textual syntax. A dimension equal to DYNAMIC_DIM renders as "?" (an
// unranked-dim placeholder) — an empty dimension list is a scalar (renders "tensor<f32>",
// no "x").
class StableHloShape {
  public:
    static constexpr std::int64_t DYNAMIC_DIM = -1;

    StableHloShape(std::vector<std::int64_t> dims, StableHloDType dtype)
        : m_dims{std::move(dims)}, m_dtype{dtype} {}

    static StableHloShape scalar(StableHloDType dtype) { return StableHloShape({}, dtype); }

    [[nodiscard]] const std::vector<std::int64_t> &get_dims() const noexcept { return m_dims; }
    [[nodiscard]] StableHloDType get_dtype() const noexcept { return m_dtype; }
    [[nodiscard]] std::size_t get_rank() const noexcept { return m_dims.size(); }

    [[nodiscard]] bool operator==(const StableHloShape &other) const noexcept {

        return m_dims == other.m_dims && m_dtype == other.m_dtype;

    }

  private:
    std::vector<std::int64_t> m_dims;
    StableHloDType m_dtype;
};

} // namespace cc::stable_hlo

// Single source of truth for the "tensor<...>" textual spelling of a shape, composing the
// std::formatter<StableHloDType> spelling for the element type. Any importer of :shape can
// std::format("{}", shape) directly.
export template <>
struct std::formatter<cc::stable_hlo::StableHloShape> {
    constexpr auto parse(std::format_parse_context &ctx) {

        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("StableHloShape: unsupported format specifier");

        }
        return it;
    }

    auto format(const cc::stable_hlo::StableHloShape &shape, std::format_context &ctx) const {

        auto out = std::format_to(ctx.out(), "tensor<");
        for (auto dim : shape.get_dims()) {
            if (dim == cc::stable_hlo::StableHloShape::DYNAMIC_DIM) {
                out = std::format_to(out, "?x");
            } else {
                out = std::format_to(out, "{}x", dim);
            }
        }
        out = std::format_to(out, "{}", shape.get_dtype());
        return std::format_to(out, ">");

    }
};

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"StableHloShape"> stable_hlo_shape_suite = [] {
    "renders a rank-2 shape"_test = [] {
        StableHloShape shape{{2, 3}, StableHloDType::f32()};
        expect(std::format("{}", shape) == "tensor<2x3xf32>");
        expect(shape.get_rank() == 2_ul);
    };

    "renders a scalar shape with no 'x' separators"_test = [] {
        auto shape = StableHloShape::scalar(StableHloDType::i32());
        expect(std::format("{}", shape) == "tensor<i32>");
        expect(shape.get_rank() == 0_ul);
    };

    "renders a dynamic dimension as '?'"_test = [] {
        StableHloShape shape{{StableHloShape::DYNAMIC_DIM, 4}, StableHloDType::f64()};
        expect(std::format("{}", shape) == "tensor<?x4xf64>");
    };

    "equality compares dims and dtype"_test = [] {
        StableHloShape left{{2, 3}, StableHloDType::f32()};
        StableHloShape right{{2, 3}, StableHloDType::f32()};
        StableHloShape different{{2, 4}, StableHloDType::f32()};
        expect(left == right);
        expect(not(left == different));
    };
};

} // namespace cc::stable_hlo::tests
#endif
