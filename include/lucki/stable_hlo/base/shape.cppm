export module yoshi_omah_lay_stable_hlo:shape;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_builder_generator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import :dtype;

export namespace cc::stable_hlo {

// A tensor shape: a dimension list plus its element type, rendered as StableHLO's
// "tensor<...>" textual syntax. A dimension equal to DYNAMIC_DIM renders as "?" (an
// unranked-dim placeholder) — an empty dimension list is a scalar (renders "tensor<f32>",
// no "x").
//
// Implements ice::builder::Attribute directly: get_full_type()/get_base_type()
// render the shape's own "tensor<...>"/element-type text, is_list() is a real answer (rank >
// 0 genuinely means "a sequence of elements along at least one dimension" — not a stub).
// get_name()/get_description() are empty: a shape has no name of its own outside a parameter
// slot, same "true in isolation" reasoning as DType's is_read_only()/is_list().
class Shape : public ice::builder::Attribute
{
public:
    static constexpr std::int64_t DYNAMIC_DIM = -1;

    Shape(std::vector<std::int64_t> dims, DType dtype) :
        m_dims{std::move(dims)},
        m_dtype{dtype}
    {
    }

    static Shape scalar(DType dtype)
    {
        return Shape({}, dtype);
    }

    [[nodiscard]] const std::vector<std::int64_t>& get_dims() const noexcept
    {
        return m_dims;
    }

    [[nodiscard]] DType get_dtype() const noexcept
    {
        return m_dtype;
    }

    [[nodiscard]] std::size_t get_rank() const noexcept
    {
        return m_dims.size();
    }

    [[nodiscard]] bool operator==(const Shape& other) const noexcept
    {
        return m_dims == other.m_dims && m_dtype == other.m_dtype;
    }

    // --- ice::builder::Attribute — see class comment ---
    ice::String get_name() const noexcept override
    {
        return ice::String{};
    }

    ice::String get_description() const noexcept override
    {
        return ice::String{};
    }

    // Renders "tensor<...>" — declared here, defined after the formatter specialization
    // below (a use before it would instantiate std::formatter's deleted primary template,
    // same reasoning as DType::to_string()).
    ice::String get_full_type() const noexcept override;

    ice::String get_base_type() const noexcept override
    {
        return ice::String{m_dtype.to_string()};
    }

    bool is_list() const noexcept override
    {
        return get_rank() > 0;
    }

private:
    std::vector<std::int64_t> m_dims;
    DType m_dtype;
};

} // namespace cc::stable_hlo

// Single source of truth for the "tensor<...>" textual spelling of a shape, composing the
// std::formatter<DType> spelling for the element type. Any importer of :shape can
// std::format("{}", shape) directly.
export template<>
struct std::formatter<cc::stable_hlo::Shape>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Shape: unsupported format specifier");
        }
        return it;
    }

    auto format(const cc::stable_hlo::Shape& shape, std::format_context& ctx) const
    {
        auto out = std::format_to(ctx.out(), "tensor<");
        for (auto dim: shape.get_dims()) {
            if (dim == cc::stable_hlo::Shape::DYNAMIC_DIM) {
                out = std::format_to(out, "?x");
            } else {
                out = std::format_to(out, "{}x", dim);
            }
        }
        out = std::format_to(out, "{}", shape.get_dtype());
        return std::format_to(out, ">");
    }
};

namespace cc::stable_hlo {

inline ice::String Shape::get_full_type() const noexcept
{
    return ice::String{std::format("{}", *this)};
}

} // namespace cc::stable_hlo

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"Shape"> stable_hlo_shape_suite = []
{
    "renders a rank-2 shape"_test = []
    {
        Shape shape{{2, 3}, DType::f32()};
        expect(std::format("{}", shape) == "tensor<2x3xf32>");
        expect(shape.get_rank() == 2_ul);
    };

    "renders a scalar shape with no 'x' separators"_test = []
    {
        auto shape = Shape::scalar(DType::i32());
        expect(std::format("{}", shape) == "tensor<i32>");
        expect(shape.get_rank() == 0_ul);
    };

    "renders a dynamic dimension as '?'"_test = []
    {
        Shape shape{{Shape::DYNAMIC_DIM, 4}, DType::f64()};
        expect(std::format("{}", shape) == "tensor<?x4xf64>");
    };

    "equality compares dims and dtype"_test = []
    {
        Shape left{{2, 3}, DType::f32()};
        Shape right{{2, 3}, DType::f32()};
        Shape different{{2, 4}, DType::f32()};
        expect(left == right);
        expect(not(left == different));
    };
};

} // namespace cc::stable_hlo::tests
#endif
