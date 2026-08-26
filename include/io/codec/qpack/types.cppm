export module io_codec_qpack:types;

import std;
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

export namespace io::codec::qpack {

enum class EncodePolicy : std::uint8_t
{
    WITH_INDEXING,
    WITHOUT_INDEXING,
    NEVER_INDEXED,
};

[[nodiscard]] constexpr EncodePolicy policy_for(std::string_view name) noexcept
{
    // Check the sensitive-header list first — any hit means the field is flagged
    // never-index before we even look at a table.
    for (auto header: NEVER_INDEXED) {
        if (name == header) {
            return EncodePolicy::NEVER_INDEXED;
        }
    }

    // No match, so this header's free to be indexed normally.
    return EncodePolicy::WITH_INDEXING;
}

enum class IndexType : std::uint8_t
{
    STATIC,
    DYNAMIC_RELATIVE,
    DYNAMIC_POST_BASE
};

} // namespace io::codec::qpack

#ifdef CONGELADO_TEST
namespace io::codec::qpack::tests {
using namespace boost::ut;

suite<"policy_for"> policy_for_suite = [] {
    "sensitive headers get flagged never-indexed"_test = [] {
        expect(policy_for("authorization") == EncodePolicy::NEVER_INDEXED);
        expect(policy_for("proxy-authorization") == EncodePolicy::NEVER_INDEXED);
        expect(policy_for("cookie") == EncodePolicy::NEVER_INDEXED);
        expect(policy_for("set-cookie") == EncodePolicy::NEVER_INDEXED);
    };

    "ordinary headers are free to be indexed"_test = [] {
        expect(policy_for("content-type") == EncodePolicy::WITH_INDEXING);
        expect(policy_for("x-custom-header") == EncodePolicy::WITH_INDEXING);
    };

    "empty name is not flagged"_test = [] {
        expect(policy_for("") == EncodePolicy::WITH_INDEXING);
    };
};

} // namespace io::codec::qpack::tests
#endif
