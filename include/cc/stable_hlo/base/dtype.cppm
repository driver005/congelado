module;

#include "c/intern/tf_datatype.h"

export module cc_stable_hlo:dtype;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace cc::stable_hlo {

// Element type for a Shape, rendered as StableHLO/MLIR textual syntax (e.g. "f32",
// "ui8", "complex<f64>"). Covers the element types actually exercised by the vendored
// stablehlo/tests/interpret/*.mlir corpus (i1/i2/.../i64, ui2/.../ui64, bf16/f16/f32/f64,
// complex<f32>/complex<f64>).
//
// Implements ice::builder::generator::TypeInfo directly — the one type-info class for all of
// stable_hlo, context-free by default (is_read_only()/is_list() are false unless set via
// with_context()) and context-aware wherever a caller actually places one in a parameter slot
// (e.g. Parameter::get_type()). get_type_attr_name() is a fixed "T" — the
// TF/MLIR schema concept of "the name of the attribute binding an operand's type" (e.g.
// Attr("T: type")), not the type's own spelling (that's get_full_type()-shaped, on
// Shape's ice::builder::generator::Attribute instead).
class DType : public ice::builder::generator::TypeInfo
{
public:
    enum class Kind : std::uint8_t
    {
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

    static constexpr DType i1() noexcept
    {
        return DType{Kind::I1};
    }

    static constexpr DType i2() noexcept
    {
        return DType{Kind::I2};
    }

    static constexpr DType i4() noexcept
    {
        return DType{Kind::I4};
    }

    static constexpr DType i8() noexcept
    {
        return DType{Kind::I8};
    }

    static constexpr DType i16() noexcept
    {
        return DType{Kind::I16};
    }

    static constexpr DType i32() noexcept
    {
        return DType{Kind::I32};
    }

    static constexpr DType i64() noexcept
    {
        return DType{Kind::I64};
    }

    static constexpr DType ui2() noexcept
    {
        return DType{Kind::UI2};
    }

    static constexpr DType ui4() noexcept
    {
        return DType{Kind::UI4};
    }

    static constexpr DType ui8() noexcept
    {
        return DType{Kind::UI8};
    }

    static constexpr DType ui16() noexcept
    {
        return DType{Kind::UI16};
    }

    static constexpr DType ui32() noexcept
    {
        return DType{Kind::UI32};
    }

    static constexpr DType ui64() noexcept
    {
        return DType{Kind::UI64};
    }

    static constexpr DType bf16() noexcept
    {
        return DType{Kind::BF16};
    }

    static constexpr DType f16() noexcept
    {
        return DType{Kind::F16};
    }

    static constexpr DType f32() noexcept
    {
        return DType{Kind::F32};
    }

    static constexpr DType f64() noexcept
    {
        return DType{Kind::F64};
    }

    static constexpr DType complex_f32() noexcept
    {
        return DType{Kind::COMPLEX_F32};
    }

    static constexpr DType complex_f64() noexcept
    {
        return DType{Kind::COMPLEX_F64};
    }

    [[nodiscard]] constexpr Kind get_kind() const noexcept
    {
        return m_kind;
    }

    // Compares by kind only — is_read_only/is_list are per-use context, not part of a type's
    // identity (Shape::operator== composes this, and two shapes with "the same"
    // element type but incidentally different context shouldn't compare unequal).
    [[nodiscard]] constexpr bool operator==(const DType& other) const noexcept
    {
        return m_kind == other.m_kind;
    }

    // Returns a copy carrying the given parameter-slot context — used wherever this dtype is
    // being handed out as an actual parameter's type (e.g.
    // Parameter::get_type()), as opposed to the context-free default.
    [[nodiscard]] constexpr DType
    with_context(bool is_read_only, bool is_list) const noexcept
    {
        DType copy = *this;
        copy.m_is_read_only = is_read_only;
        copy.m_is_list = is_list;
        return copy;
    }

    // --- ice::builder::generator::TypeInfo — see class comment ---

    // Maps this element type onto the shared TF_DataType vocabulary the C ABI already carries
    // (c/intern/tf_datatype.h) — e.g. F32 -> TF_FLOAT, I32 -> TF_INT32 — instead of leaking
    // Kind's own internal ordering across the ABI, which would mean nothing to a caller on the
    // other side. I1 maps to TF_BOOL: StableHLO's i1 is its boolean/predicate type, not a
    // general 1-bit int, and TF_DataType has no bare "1-bit int" entry.
    int get_data_type() const override
    {

        switch (m_kind) {
            case Kind::I1: return TF_BOOL;
            case Kind::I2: return TF_INT2;
            case Kind::I4: return TF_INT4;
            case Kind::I8: return TF_INT8;
            case Kind::I16: return TF_INT16;
            case Kind::I32: return TF_INT32;
            case Kind::I64: return TF_INT64;
            case Kind::UI2: return TF_UINT2;
            case Kind::UI4: return TF_UINT4;
            case Kind::UI8: return TF_UINT8;
            case Kind::UI16: return TF_UINT16;
            case Kind::UI32: return TF_UINT32;
            case Kind::UI64: return TF_UINT64;
            case Kind::BF16: return TF_BFLOAT16;
            case Kind::F16: return TF_HALF;
            case Kind::F32: return TF_FLOAT;
            case Kind::F64: return TF_DOUBLE;
            case Kind::COMPLEX_F32: return TF_COMPLEX64;
            case Kind::COMPLEX_F64: return TF_COMPLEX128;
        }
        return 0; // unreachable — every Kind has a mapping
    }

    ice::sonic::StringRuntime get_type_attr_name() const override
    {
        return ice::sonic::StringRuntime{std::string{"T"}};
    }

    bool is_read_only() const override
    {
        return m_is_read_only;
    }

    bool is_list() const override
    {
        return m_is_list;
    }

    // Renders this element type exactly as StableHLO's textual assembly spells it, e.g.
    // "f32", "ui8", "complex<f64>" — see stablehlo/tests/interpret/add.mlir for the
    // canonical spellings this mirrors. The spelling table lives in the
    // std::formatter<DType> specialization below; to_string() is a thin
    // convenience wrapper so callers can also std::format("{}", dtype) directly.
    // Defined out-of-line (after the formatter) so the std::format call inside resolves
    // against that specialization — a use before the specialization would instantiate
    // std::formatter's deleted primary template.
    [[nodiscard]] std::string to_string() const;

private:
    constexpr explicit DType(Kind kind) noexcept :
        m_kind{kind}
    {
    }

    Kind m_kind;
    bool m_is_read_only{false};
    bool m_is_list{false};
};

} // namespace cc::stable_hlo

// Single source of truth for StableHLO/MLIR textual spellings of element types ("f32",
// "ui8", "complex<f64>", ...). DType::to_string() delegates here, and any
// std::format("{}", dtype) call resolves against this specialization too.
export template<>
struct std::formatter<cc::stable_hlo::DType>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("DType: unsupported format specifier");
        }
        return it;
    }

    auto format(const cc::stable_hlo::DType& dtype, std::format_context& ctx) const
    {
        using Kind = cc::stable_hlo::DType::Kind;
        std::string_view text;
        switch (dtype.get_kind()) {
            case Kind::I1:
                text = "i1";
                break;
            case Kind::I2:
                text = "i2";
                break;
            case Kind::I4:
                text = "i4";
                break;
            case Kind::I8:
                text = "i8";
                break;
            case Kind::I16:
                text = "i16";
                break;
            case Kind::I32:
                text = "i32";
                break;
            case Kind::I64:
                text = "i64";
                break;
            case Kind::UI2:
                text = "ui2";
                break;
            case Kind::UI4:
                text = "ui4";
                break;
            case Kind::UI8:
                text = "ui8";
                break;
            case Kind::UI16:
                text = "ui16";
                break;
            case Kind::UI32:
                text = "ui32";
                break;
            case Kind::UI64:
                text = "ui64";
                break;
            case Kind::BF16:
                text = "bf16";
                break;
            case Kind::F16:
                text = "f16";
                break;
            case Kind::F32:
                text = "f32";
                break;
            case Kind::F64:
                text = "f64";
                break;
            case Kind::COMPLEX_F32:
                return std::format_to(ctx.out(), "complex<f32>");
            case Kind::COMPLEX_F64:
                return std::format_to(ctx.out(), "complex<f64>");
        }
        return std::format_to(ctx.out(), "{}", text);
    }
};

namespace cc::stable_hlo {

inline std::string DType::to_string() const
{
    return std::format("{}", *this);
}

} // namespace cc::stable_hlo

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"DType"> stable_hlo_dtype_suite = [] {
    "renders integer and float spellings"_test = [] {
        expect(std::format("{}", DType::i1()) == "i1");
        expect(std::format("{}", DType::i32()) == "i32");
        expect(std::format("{}", DType::ui8()) == "ui8");
        expect(std::format("{}", DType::f32()) == "f32");
        expect(std::format("{}", DType::f64()) == "f64");
        expect(std::format("{}", DType::complex_f32()) == "complex<f32>");
    };

    "equality compares by kind"_test = [] {
        expect(DType::f32() == DType::f32());
        expect(not(DType::f32() == DType::f64()));
    };
};

} // namespace cc::stable_hlo::tests
#endif
