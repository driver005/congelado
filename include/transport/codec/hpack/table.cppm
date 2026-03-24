export module hpack:table;
import std;
import :consts;
import shared;

export namespace codec::hpack::table {

inline constexpr std::array<std::pair<std::string_view, std::string_view>, 61> k_static_table = {{
    /* 0  */ {":authority", ""},
    /* 1  */ {":method", "GET"},
    /* 2  */ {":method", "POST"},
    /* 3  */ {":path", "/"},
    /* 4  */ {":path", "/index.html"},
    /* 5  */ {":scheme", "http"},
    /* 6  */ {":scheme", "https"},
    /* 7  */ {":status", "200"},
    /* 8  */ {":status", "204"},
    /* 9  */ {":status", "206"},
    /* 10 */ {":status", "304"},
    /* 11 */ {":status", "400"},
    /* 12 */ {":status", "404"},
    /* 13 */ {":status", "500"},
    /* 14 */ {"accept-charset", ""},
    /* 15 */ {"accept-encoding", "gzip, deflate"},
    /* 16 */ {"accept-language", ""},
    /* 17 */ {"accept-ranges", ""},
    /* 18 */ {"accept", ""},
    /* 19 */ {"access-control-allow-origin", ""},
    /* 20 */ {"age", ""},
    /* 21 */ {"allow", ""},
    /* 22 */ {"authorization", ""},
    /* 23 */ {"cache-control", ""},
    /* 24 */ {"content-disposition", ""},
    /* 25 */ {"content-encoding", ""},
    /* 26 */ {"content-language", ""},
    /* 27 */ {"content-length", ""},
    /* 28 */ {"content-location", ""},
    /* 29 */ {"content-range", ""},
    /* 30 */ {"content-type", ""},
    /* 31 */ {"cookie", ""},
    /* 32 */ {"date", ""},
    /* 33 */ {"etag", ""},
    /* 34 */ {"expect", ""},
    /* 35 */ {"expires", ""},
    /* 36 */ {"from", ""},
    /* 37 */ {"host", ""},
    /* 38 */ {"if-match", ""},
    /* 39 */ {"if-modified-since", ""},
    /* 40 */ {"if-none-match", ""},
    /* 41 */ {"if-range", ""},
    /* 42 */ {"if-unmodified-since", ""},
    /* 43 */ {"last-modified", ""},
    /* 44 */ {"link", ""},
    /* 45 */ {"location", ""},
    /* 46 */ {"max-forwards", ""},
    /* 47 */ {"proxy-authenticate", ""},
    /* 48 */ {"proxy-authorization", ""},
    /* 49 */ {"range", ""},
    /* 50 */ {"referer", ""},
    /* 51 */ {"refresh", ""},
    /* 52 */ {"retry-after", ""},
    /* 53 */ {"server", ""},
    /* 54 */ {"set-cookie", ""},
    /* 55 */ {"strict-transport-security", ""},
    /* 56 */ {"transfer-encoding", ""},
    /* 57 */ {"user-agent", ""},
    /* 58 */ {"vary", ""},
    /* 59 */ {"via", ""},
    /* 60 */ {"www-authenticate", ""},
}};

using HpackStatic = shared::table::StaticTable<k_static_table>;

// ─────────────────────────────────────────────────────────────────────────────
// HeaderTable — RFC 7541 unified index space
//
// search() returns a SearchResult (std::size_t) where shared::table::sr_index() is the
// 1-based RFC index into the unified static + dynamic space.
//
// Static table results: shared::table::sr_index() in [1..61]
// Dynamic table results: shared::table::sr_index() in [62..]
// ─────────────────────────────────────────────────────────────────────────────
class HeaderTable {
  public:
    explicit HeaderTable(std::size_t max_size = DEFAULT_MAX_TABLE_SIZE) : m_dyn{max_size} {}

    // ── Lookup by 1-based RFC index ───────────────────────────────────────────

    [[nodiscard]] std::optional<std::pair<std::string_view, std::string_view>>
    operator[](std::size_t idx) const noexcept {
        if (idx == 0)
            return std::nullopt;
        if (idx <= HpackStatic::STATIC_SIZE)
            return HpackStatic::at(idx - 1);
        if (const auto *e = m_dyn.at_pos(idx - HpackStatic::STATIC_SIZE - 1))
            return std::make_pair(e->name(), e->value());
        return std::nullopt;
    }

    [[nodiscard]] std::pair<std::string_view, std::string_view> at(std::size_t idx) const {
        if (auto r = (*this)[idx])
            return *r;
        throw std::out_of_range{"hpack::HeaderTable: invalid index"};
    }

    // ── Search ────────────────────────────────────────────────────────────────
    // Returns SearchResult{} if not found.
    // sr_index() = 1-based RFC index on hit.
    // sr_is_static() / shared::table::sr_full_match() describe the match quality.
    //
    // Priority: static exact > dynamic exact > static name > dynamic name.

    [[nodiscard]] shared::SearchResult search(std::string_view name, std::string_view value) const noexcept {
        // 1. Static exact → sr_index = orig_idx + 1 (0→1-based)
        if (auto r = HpackStatic::find(name, value))
            return shared::SearchResult{
                r.index() + 1,
                true,
                true,
            };

        // 2. Dynamic exact → sr_index = STATIC_SIZE + deque_pos + 1
        if (auto r = m_dyn.search(name, value); r.found() && r.is_full_match()) {
            // sr_index(r) from DynamicTable is abs = gen-1
            // gen = abs + 1, pos = gen_to_pos(gen)
            const std::size_t pos = m_dyn.gen_to_pos(r.index() + 1);
            return shared::SearchResult{
                HpackStatic::STATIC_SIZE + pos + 1,
                false,
                true,
            };
        }

        // 3. Static name-only
        if (auto r = HpackStatic::find_name(name))
            return shared::SearchResult{
                r.index() + 1,
                true,
                false,
            };

        // 4. Dynamic name-only
        if (auto r = m_dyn.search(name, {}); r.found()) {
            const std::size_t pos = m_dyn.gen_to_pos(r.index() + 1);
            return shared::SearchResult{
                HpackStatic::STATIC_SIZE + pos + 1,
                false,
                false,
            };
        }

        return shared::SearchResult{};
    }

    // ── Mutation ──────────────────────────────────────────────────────────────

    void insert(std::string name, std::string value) { m_dyn.insert(std::move(name), std::move(value)); }

    void set_max_size(std::size_t new_max) { m_dyn.set_max_size(new_max); }

    // ── Accessors ─────────────────────────────────────────────────────────────

    [[nodiscard]] std::size_t max_size() const noexcept { return m_dyn.max_size(); }
    [[nodiscard]] std::size_t current_size() const noexcept { return m_dyn.current_size(); }
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.size(); }
    [[nodiscard]] std::size_t total_entries() const noexcept { return HpackStatic::STATIC_SIZE + m_dyn.size(); }

  private:
    shared::table::DynamicTable m_dyn;
};

} // namespace codec::hpack::table
