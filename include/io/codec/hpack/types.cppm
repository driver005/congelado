module;
#include <type_traits>
export module io_codec_hpack:types;
import std;
import io_error;
import :consts;

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
