module;
#include <type_traits>
export module hpack:types;
import std;
import error;
import :consts;

// Some header that are recommended
constexpr std::string_view NEVER_INDEXED[] = {
    "authorization",
    "proxy-authorization",
    "cookie",
    "set-cookie",
};

export namespace codec::hpack {

struct DecodingError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

enum class EncodePolicy : std::uint8_t {
    WithIndexing,
    WithoutIndexing,
    NeverIndexed,
};

[[nodiscard]] constexpr EncodePolicy policy_for(std::string_view name) noexcept {
    for (auto s : NEVER_INDEXED)
        if (name == s)
            return EncodePolicy::NeverIndexed;

    return EncodePolicy::WithIndexing;
}
} // namespace codec::hpack
