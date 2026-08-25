module;
#include <type_traits>
export module io_codec_hpack:types;
import std;
import io_error;
import :consts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// Some header that are recommended
constexpr std::string_view NEVER_INDEXED[] = {
    "authorization",
    "proxy-authorization",
    "cookie",
    "set-cookie",
};

export namespace io::codec::hpack {

enum class EncodePolicy : std::uint8_t {
    WITH_INDEXING,
    WITHOUT_INDEXING,
    NEVER_INDEXED,
};

[[nodiscard]] constexpr EncodePolicy policy_for(std::string_view name) noexcept {
    // Walk the sensitive-header list — an exact match means this one's never getting
    // cached in the dynamic table, no cap.
    for (auto header : NEVER_INDEXED) {
        if (name == header) {
            return EncodePolicy::NEVER_INDEXED;
        }
    }

    // Nothing sensitive matched, so default to normal indexing.
    return EncodePolicy::WITH_INDEXING;
}
} // namespace codec::hpack

#ifdef CONGELADO_TEST
namespace io::codec::hpack::tests {
using namespace boost::ut;

suite<"policy_for"> policy_for_suite = [] {
    "sensitive headers get NEVER_INDEXED"_test = [] {
        expect(policy_for("authorization") == EncodePolicy::NEVER_INDEXED);
        expect(policy_for("proxy-authorization") == EncodePolicy::NEVER_INDEXED);
        expect(policy_for("cookie") == EncodePolicy::NEVER_INDEXED);
        expect(policy_for("set-cookie") == EncodePolicy::NEVER_INDEXED);
    };

    "ordinary headers default to WITH_INDEXING"_test = [] {
        expect(policy_for("content-type") == EncodePolicy::WITH_INDEXING);
        expect(policy_for("") == EncodePolicy::WITH_INDEXING);
    };
};

} // namespace io::codec::hpack::tests
#endif
