export module qpack:table;

import std;
import shared;

export namespace codec::quic::qpack {

inline constexpr std::array<std::pair<std::string_view, std::string_view>, 99> k_static_table = {{
    /* 0  */ {":authority", ""},
    /* 1  */ {":path", "/"},
    /* 2  */ {"age", "0"},
    /* 3  */ {"content-disposition", ""},
    /* 4  */ {"content-length", "0"},
    /* 5  */ {"cookie", ""},
    /* 6  */ {"date", ""},
    /* 7  */ {"etag", ""},
    /* 8  */ {"if-modified-since", ""},
    /* 9  */ {"if-none-match", ""},
    /* 10 */ {"last-modified", ""},
    /* 11 */ {"link", ""},
    /* 12 */ {"location", ""},
    /* 13 */ {"referer", ""},
    /* 14 */ {"set-cookie", ""},
    /* 15 */ {":method", "CONNECT"},
    /* 16 */ {":method", "DELETE"},
    /* 17 */ {":method", "GET"},
    /* 18 */ {":method", "HEAD"},
    /* 19 */ {":method", "OPTIONS"},
    /* 20 */ {":method", "POST"},
    /* 21 */ {":method", "PUT"},
    /* 22 */ {":scheme", "http"},
    /* 23 */ {":scheme", "https"},
    /* 24 */ {":status", "103"},
    /* 25 */ {":status", "200"},
    /* 26 */ {":status", "304"},
    /* 27 */ {":status", "404"},
    /* 28 */ {":status", "503"},
    /* 29 */ {"accept", "*/*"},
    /* 30 */ {"accept", "application/dns-message"},
    /* 31 */ {"accept-encoding", "gzip, deflate, br"},
    /* 32 */ {"accept-ranges", "bytes"},
    /* 33 */ {"access-control-allow-headers", "cache-control"},
    /* 34 */ {"access-control-allow-headers", "content-type"},
    /* 35 */ {"access-control-allow-origin", "*"},
    /* 36 */ {"cache-control", "max-age=0"},
    /* 37 */ {"cache-control", "max-age=2592000"},
    /* 38 */ {"cache-control", "max-age=604800"},
    /* 39 */ {"cache-control", "no-cache"},
    /* 40 */ {"cache-control", "no-store"},
    /* 41 */ {"cache-control", "public, max-age=31536000"},
    /* 42 */ {"content-encoding", "br"},
    /* 43 */ {"content-encoding", "gzip"},
    /* 44 */ {"content-type", "application/dns-message"},
    /* 45 */ {"content-type", "application/javascript"},
    /* 46 */ {"content-type", "application/json"},
    /* 47 */ {"content-type", "application/x-www-form-urlencoded"},
    /* 48 */ {"content-type", "image/gif"},
    /* 49 */ {"content-type", "image/jpeg"},
    /* 50 */ {"content-type", "image/png"},
    /* 51 */ {"content-type", "text/css"},
    /* 52 */ {"content-type", "text/html; charset=utf-8"},
    /* 53 */ {"content-type", "text/plain"},
    /* 54 */ {"content-type", "text/plain;charset=utf-8"},
    /* 55 */ {"range", "bytes=0-"},
    /* 56 */ {"strict-transport-security", "max-age=31536000"},
    /* 57 */ {"strict-transport-security", "max-age=31536000; includesubdomains"},
    /* 58 */ {"strict-transport-security", "max-age=31536000; includesubdomains; preload"},
    /* 59 */ {"vary", "accept-encoding"},
    /* 60 */ {"vary", "origin"},
    /* 61 */ {"x-content-type-options", "nosniff"},
    /* 62 */ {"x-xss-protection", "1; mode=block"},
    /* 63 */ {":status", "100"},
    /* 64 */ {":status", "204"},
    /* 65 */ {":status", "206"},
    /* 66 */ {":status", "302"},
    /* 67 */ {":status", "400"},
    /* 68 */ {":status", "403"},
    /* 69 */ {":status", "421"},
    /* 70 */ {":status", "425"},
    /* 71 */ {":status", "500"},
    /* 72 */ {"accept-language", ""},
    /* 73 */ {"access-control-allow-credentials", "FALSE"},
    /* 74 */ {"access-control-allow-credentials", "TRUE"},
    /* 75 */ {"access-control-allow-headers", "*"},
    /* 76 */ {"access-control-allow-methods", "get"},
    /* 77 */ {"access-control-allow-methods", "get, post, options"},
    /* 78 */ {"access-control-allow-methods", "options"},
    /* 79 */ {"access-control-expose-headers", "content-length"},
    /* 80 */ {"access-control-request-headers", "content-type"},
    /* 81 */ {"access-control-request-method", "get"},
    /* 82 */ {"access-control-request-method", "post"},
    /* 83 */ {"alt-svc", "clear"},
    /* 84 */ {"authorization", ""},
    /* 85 */ {"content-security-policy", "script-src 'none'; object-src 'none'; base-uri 'none'"},
    /* 86 */ {"early-data", "1"},
    /* 87 */ {"expect-ct", ""},
    /* 88 */ {"forwarded", ""},
    /* 89 */ {"if-range", ""},
    /* 90 */ {"origin", ""},
    /* 91 */ {"purpose", "prefetch"},
    /* 92 */ {"server", ""},
    /* 93 */ {"timing-allow-origin", "*"},
    /* 94 */ {"upgrade-insecure-requests", "1"},
    /* 95 */ {"user-agent", ""},
    /* 96 */ {"x-forwarded-for", ""},
    /* 97 */ {"x-frame-options", "deny"},
    /* 98 */ {"x-frame-options", "sameorigin"},
}};

using QpackStatic = shared::table::StaticTable<k_static_table>;

// HeaderTable — RFC 9204 separate index spaces
//
// search() returns a SearchResult (std::size_t) where:
//   sr_is_static() = true  → sr_index() is 0-based static table index
//   sr_is_static() = false → sr_index() is absolute dynamic index (gen - 1)
//
// Callers convert to relative / post-base index using the helpers below.
class HeaderTable {
  public:
    explicit HeaderTable(std::size_t max_capacity = 0) : m_dyn{max_capacity} {}

    [[nodiscard]] static std::optional<std::pair<std::string_view, std::string_view>>
    static_at(std::size_t idx) noexcept {
        return QpackStatic::at(idx);
    }

    [[nodiscard]] std::optional<std::pair<std::string_view, std::string_view>>
    dyn_at_abs(std::size_t abs) const noexcept {
        if (const auto *e = m_dyn.at_gen(static_cast<std::uint64_t>(abs) + 1))
            return std::make_pair(e->name(), e->value());
        return std::nullopt;
    }

    // ── Search ────────────────────────────────────────────────────────────────
    // Priority: static exact > dynamic exact > static name > dynamic name.
    // sr_index() carries the protocol-native index directly:
    //   static  → 0-based orig_idx (pass straight to encoder as T=1 ref)
    //   dynamic → absolute index   (convert to relative/post-base as needed)

    [[nodiscard]] shared::SearchResult search(std::string_view name, std::string_view value) const noexcept {
        if (auto r = QpackStatic::find(name, value))
            return r; // static exact, idx=orig_idx
        if (auto r = m_dyn.search(name, value); r.found() && r.is_full_match())
            return r; // dynamic exact, idx=abs
        if (auto r = QpackStatic::find_name(name))
            return r; // static name, idx=orig_idx
        if (auto r = m_dyn.search(name, {}); r.found())
            return r; // dynamic name, idx=abs
        return shared::SearchResult{};
    }

    // relative = insert_count - 1 - abs
    [[nodiscard]] std::size_t abs_to_rel(std::size_t abs) const noexcept {
        const std::uint64_t ic = m_dyn.insert_count();
        if (abs >= ic)
            return shared::SIZE_MAX;
        return static_cast<std::size_t>(ic - 1 - abs);
    }

    [[nodiscard]] std::size_t rel_to_abs(std::size_t rel) const noexcept {
        const std::uint64_t ic = m_dyn.insert_count();
        if (ic == 0 || rel >= ic)
            return shared::SIZE_MAX;
        return static_cast<std::size_t>(ic - 1 - rel);
    }

    // post-base: abs = base + 1 + pb
    [[nodiscard]] static std::size_t post_base_to_abs(std::size_t base, std::size_t pb) noexcept {
        return base + 1 + pb;
    }

    [[nodiscard]] static std::size_t abs_to_post_base(std::size_t base, std::size_t abs) noexcept {
        if (abs <= base)
            return shared::SIZE_MAX;
        return abs - base - 1;
    }

    [[nodiscard]] std::size_t encode_ric(std::size_t ric) const noexcept {
        if (ric == 0)
            return 0;
        const std::size_t me = std::max(m_dyn.max_size() / 32, std::size_t{1});
        return (ric % (2 * me)) + 1;
    }

    [[nodiscard]] std::size_t decode_ric(std::size_t enc) const noexcept {
        if (enc == 0)
            return 0;
        const std::size_t me = std::max(m_dyn.max_size() / 32, std::size_t{1});
        const std::size_t full = 2 * me;
        const std::size_t maxv = static_cast<std::size_t>(m_dyn.insert_count()) + me;
        std::size_t ric = (maxv / full) * full + enc - 1;
        if (ric > maxv)
            ric -= full;
        return ric;
    }

    [[nodiscard]] bool is_ready(std::size_t ric) const noexcept {
        return ric <= static_cast<std::size_t>(m_dyn.insert_count());
    }

    // Returns absolute index of inserted entry, shared::SIZE_MAX if too large.
    std::size_t insert(std::string name, std::string value) {
        const std::uint64_t gen = m_dyn.insert(std::move(name), std::move(value));
        return gen == 0 ? shared::SIZE_MAX : static_cast<std::size_t>(gen - 1);
    }

    void set_capacity(std::size_t cap) { m_dyn.set_max_size(cap); }

    void block_stream(std::size_t stream_id, std::size_t ric) { m_blocked.push_back({stream_id, ric}); }

    // Returns stream_ids now unblocked. Removes them from the list.
    std::vector<std::size_t> drain_unblocked() {
        std::vector<std::size_t> ready;
        const std::size_t ic = static_cast<std::size_t>(m_dyn.insert_count());
        auto it = std::remove_if(m_blocked.begin(), m_blocked.end(), [ic, &ready](const BlockedStream &b) {
            if (b.ric <= ic) {
                ready.push_back(b.stream_id);
                return true;
            }
            return false;
        });
        m_blocked.erase(it, m_blocked.end());
        return ready;
    }

    [[nodiscard]] std::size_t insert_count() const noexcept { return static_cast<std::size_t>(m_dyn.insert_count()); }
    [[nodiscard]] std::size_t capacity() const noexcept { return m_dyn.max_size(); }
    [[nodiscard]] std::size_t used() const noexcept { return m_dyn.current_size(); }
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.size(); }
    [[nodiscard]] std::size_t blocked_count() const noexcept { return m_blocked.size(); }

  private:
    struct BlockedStream {
        std::size_t stream_id;
        std::size_t ric;
    };

    shared::table::DynamicTable m_dyn;
    std::vector<BlockedStream> m_blocked;
};

} // namespace codec::quic::qpack
