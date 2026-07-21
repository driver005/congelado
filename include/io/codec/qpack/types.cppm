export module io_codec_qpack:types;

import std;

// Some header that are recommended
constexpr std::string_view NEVER_INDEXED[] = {
    "authorization",
    "proxy-authorization",
    "cookie",
    "set-cookie",
};

export namespace io::codec::qpack {

enum class EncodePolicy : std::uint8_t {
    WITH_INDEXING,
    WITHOUT_INDEXING,
    NEVER_INDEXED,
};

[[nodiscard]] constexpr EncodePolicy policy_for(std::string_view name) noexcept {
    // Check the sensitive-header list first — any hit means the field is flagged
    // never-index before we even look at a table.
    for (auto header : NEVER_INDEXED) {
        if (name == header) {
            return EncodePolicy::NEVER_INDEXED;
        }
    }

    // No match, so this header's free to be indexed normally.
    return EncodePolicy::WITH_INDEXING;
}

enum class IndexType : std::uint8_t { STATIC, DYNAMIC_RELATIVE, DYNAMIC_POST_BASE };

} // namespace io::codec::qpack
