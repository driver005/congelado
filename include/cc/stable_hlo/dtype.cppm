export module cc_stable_hlo:dtype;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace cc::stable_hlo {

// Element type for a StableHloShape, rendered as StableHLO/MLIR textual syntax (e.g. "f32",
// "ui8", "complex<f64>"). Covers the element types actually exercised by the vendored
// stablehlo/tests/interpret/*.mlir corpus (i1/i2/.../i64, ui2/.../ui64, bf16/f16/f32/f64,
// complex<f32>/complex<f64>).
class StableHloDType {
  public:
    enum class Kind : std::uint8_t {
        I1,
        I2,
        I4,
        I8,
        I16,
        I32,
        I64,
        UI2,
        UI4,
        UI8,
        UI16,
        UI32,
        UI64,
        BF16,
        F16,
        F32,
        F64,
        COMPLEX_F32,
        COMPLEX_F64,
    };

    static constexpr StableHloDType i1() noexcept { return StableHloDType{Kind::I1}; }
    static constexpr StableHloDType i2() noexcept { return StableHloDType{Kind::I2}; }
    static constexpr StableHloDType i4() noexcept { return StableHloDType{Kind::I4}; }
    static constexpr StableHloDType i8() noexcept { return StableHloDType{Kind::I8}; }
    static constexpr StableHloDType i16() noexcept { return StableHloDType{Kind::I16}; }
    static constexpr StableHloDType i32() noexcept { return StableHloDType{Kind::I32}; }
    static constexpr StableHloDType i64() noexcept { return StableHloDType{Kind::I64}; }
    static constexpr StableHloDType ui2() noexcept { return StableHloDType{Kind::UI2}; }
    static constexpr StableHloDType ui4() noexcept { return StableHloDType{Kind::UI4}; }
    static constexpr StableHloDType ui8() noexcept { return StableHloDType{Kind::UI8}; }
    static constexpr StableHloDType ui16() noexcept { return StableHloDType{Kind::UI16}; }
    static constexpr StableHloDType ui32() noexcept { return StableHloDType{Kind::UI32}; }
    static constexpr StableHloDType ui64() noexcept { return StableHloDType{Kind::UI64}; }
    static constexpr StableHloDType bf16() noexcept { return StableHloDType{Kind::BF16}; }
    static constexpr StableHloDType f16() noexcept { return StableHloDType{Kind::F16}; }
    static constexpr StableHloDType f32() noexcept { return StableHloDType{Kind::F32}; }
    static constexpr StableHloDType f64() noexcept { return StableHloDType{Kind::F64}; }
    static constexpr StableHloDType complex_f32() noexcept { return StableHloDType{Kind::COMPLEX_F32}; }
    static constexpr StableHloDType complex_f64() noexcept { return StableHloDType{Kind::COMPLEX_F64}; }

    [[nodiscard]] constexpr Kind get_kind() const noexcept { return m_kind; }

    [[nodiscard]] constexpr bool operator==(const StableHloDType &other) const noexcept {

        return m_kind == other.m_kind;

    }

    // Renders this element type exactly as StableHLO's textual assembly spells it, e.g.
    // "f32", "ui8", "complex<f64>" — see stablehlo/tests/interpret/add.mlir for the
    // canonical spellings this mirrors. The spelling table lives in the
    // std::formatter<StableHloDType> specialization below; this method is a thin
    // convenience wrapper so callers can also std::format("{}", dtype) directly.
    // Defined out-of-line (after the formatter) so the std::format call inside resolves
    // against that specialization — a use before the specialization would instantiate
    // std::formatter's deleted primary template.
    [[nodiscard]] std::string to_mlir_text() const;

  private:
    constexpr explicit StableHloDType(Kind kind) noexcept : m_kind{kind} {}

    Kind m_kind;
};

} // namespace cc::stable_hlo

// Single source of truth for StableHLO/MLIR textual spellings of element types ("f32",
// "ui8", "complex<f64>", ...). StableHloDType::to_mlir_text() delegates here, and any
// std::format("{}", dtype) call resolves against this specialization too.
export template <>
struct std::formatter<cc::stable_hlo::StableHloDType> {
    constexpr auto parse(std::format_parse_context &ctx) {

        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("StableHloDType: unsupported format specifier");

        }
        return it;
    }

    auto format(const cc::stable_hlo::StableHloDType &dtype, std::format_context &ctx) const {

        using Kind = cc::stable_hlo::StableHloDType::Kind;
        std::string_view text;
        switch (dtype.get_kind()) {
            case Kind::I1: text = "i1"; break;
            case Kind::I2: text = "i2"; break;
            case Kind::I4: text = "i4"; break;
            case Kind::I8: text = "i8"; break;
            case Kind::I16: text = "i16"; break;
            case Kind::I32: text = "i32"; break;
            case Kind::I64: text = "i64"; break;
            case Kind::UI2: text = "ui2"; break;
            case Kind::UI4: text = "ui4"; break;
            case Kind::UI8: text = "ui8"; break;
            case Kind::UI16: text = "ui16"; break;
            case Kind::UI32: text = "ui32"; break;
            case Kind::UI64: text = "ui64"; break;
            case Kind::BF16: text = "bf16"; break;
            case Kind::F16: text = "f16"; break;
            case Kind::F32: text = "f32"; break;
            case Kind::F64: text = "f64"; break;
            case Kind::COMPLEX_F32: return std::format_to(ctx.out(), "complex<f32>");
            case Kind::COMPLEX_F64: return std::format_to(ctx.out(), "complex<f64>");
        }
        return std::format_to(ctx.out(), "{}", text);

    }
};

namespace cc::stable_hlo {

inline std::string StableHloDType::to_mlir_text() const { return std::format("{}", *this); }

} // namespace cc::stable_hlo

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"StableHloDType"> stable_hlo_dtype_suite = [] {
    "renders integer and float spellings"_test = [] {
        expect(std::format("{}", StableHloDType::i1()) == "i1");
        expect(std::format("{}", StableHloDType::i32()) == "i32");
        expect(std::format("{}", StableHloDType::ui8()) == "ui8");
        expect(std::format("{}", StableHloDType::f32()) == "f32");
        expect(std::format("{}", StableHloDType::f64()) == "f64");
        expect(std::format("{}", StableHloDType::complex_f32()) == "complex<f32>");
    };

    "equality compares by kind"_test = [] {
        expect(StableHloDType::f32() == StableHloDType::f32());
        expect(not(StableHloDType::f32() == StableHloDType::f64()));
    };
};

} // namespace cc::stable_hlo::tests
#endif
