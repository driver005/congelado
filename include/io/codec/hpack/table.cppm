export module io_codec_hpack:table;

import std;
import io_codec_shared;
import interfaces;
import :consts;

export namespace io::codec::hpack {

// TODO: make length a constant

// FIXME(clang-tidy): bugprone-throwing-static-initialization — STATIC_TABLE's shared_ptr/
// make_shared init can throw (bad_alloc), and it's used below as a non-type template argument
// to shared_codec::table::StaticTable<STATIC_TABLE>, so it can't be turned into a lazily
// initialized function-local static without a wider API change across this module.
inline const std::array<std::shared_ptr<interfaces::io::HeaderField<true>>, 61> STATIC_TABLE = {
    /* 0  */ std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::AUTHORITY, ""),
    /* 1  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD,
                                                        "GET"),
    /* 2  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD,
                                                        "POST"),
    /* 3  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::PATH, "/"),
    /* 4  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::PATH,
                                                        "/index.html"),
    /* 5  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SCHEME,
                                                        "http"),
    /* 6  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SCHEME,
                                                        "https"),
    /* 7  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "200"),
    /* 8  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "204"),
    /* 9  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "206"),
    /* 10 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "304"),
    /* 11 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "400"),
    /* 12 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "404"),
    /* 13 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS,
                                                        "500"),
    /* 14 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCEPT_CHARSET, ""),
    /* 15 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCEPT_ENCODING, "gzip, deflate"),
    /* 16 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCEPT_LANGUAGE, ""),
    /* 17 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT_RANGES,
                                                        ""),
    /* 18 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT, ""),
    /* 19 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_ORIGIN, ""),
    /* 20 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::AGE, ""),
    /* 21 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ALLOW, ""),
    /* 22 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::AUTHORIZATION,
                                                        ""),
    /* 23 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                        ""),
    /* 24 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_DISPOSITION, ""),
    /* 25 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_ENCODING, ""),
    /* 26 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_LANGUAGE, ""),
    /* 27 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_LENGTH, ""),
    /* 28 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_LOCATION, ""),
    /* 29 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_RANGE,
                                                        ""),
    /* 30 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                        ""),
    /* 31 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::COOKIE, ""),
    /* 32 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::DATE, ""),
    /* 33 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::E_TAG, ""),
    /* 34 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::EXPECT, ""),
    /* 35 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::EXPIRES, ""),
    /* 36 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::FROM, ""),
    /* 37 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::HOST, ""),
    /* 38 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::IF_MATCH, ""),
    /* 39 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::IF_MODIFIED_SINCE, ""),
    /* 40 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::IF_NONE_MATCH,
                                                        ""),
    /* 41 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::IF_RANGE, ""),
    /* 42 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::IF_UNMODIFIED_SINCE, ""),
    /* 43 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::LAST_MODIFIED,
                                                        ""),
    /* 44 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::LINK, ""),
    /* 45 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::LOCATION, ""),
    /* 46 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::MAX_FORWARDS,
                                                        ""),
    /* 47 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::PROXY_AUTHENTICATE, ""),
    /* 48 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::PROXY_AUTHORIZATION, ""),
    /* 49 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::RANGE, ""),
    /* 50 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::REFERER, ""),
    /* 51 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::REFRESH, ""),
    /* 52 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::RETRY_AFTER,
                                                        ""),
    /* 53 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SERVER, ""),
    /* 54 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SET_COOKIE,
                                                        ""),
    /* 55 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::STRICT_TRANSPORT_SECURITY, ""),
    /* 56 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::TRANSFER_ENCODING, ""),
    /* 57 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::USER_AGENT,
                                                        ""),
    /* 58 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::VARY, ""),
    /* 59 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::VIA, ""),
    /* 60 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::WWW_AUTHENTICATE, ""),
};

using HPackStatic = shared_codec::table::StaticTable<STATIC_TABLE>;

// HeaderTable — RFC 7541 unified index space
class HPackTable {
  public:
    /**
     * @brief Spins up an HPACK header table — the RFC 7541 static table is baked in for free,
     * the dynamic table starts empty with `max_size` bytes to play with.
     * @param max_size the dynamic table's byte budget (name + value + 32 bytes overhead per
     * RFC 7541 §4.1 per live entry), defaults to DEFAULT_MAX_TABLE_SIZE.
     */
    explicit HPackTable(std::size_t max_size = DEFAULT_MAX_TABLE_SIZE) : m_dyn{max_size} {}

    /**
     * @brief Resolves a 1-based HPACK index into its header, walking RFC 7541's unified index
     * space — static entries (1..61) first, dynamic entries right after. No throwing, just a
     * clean miss on bad input.
     * @param idx the 1-based HPACK index; 0 is never valid in the spec.
     * @return the resolved header entry, or std::nullopt if `idx` is 0 or out of range for both
     * tables.
     */
    [[nodiscard]] std::optional<interfaces::io::HeaderEntry>
    operator[](std::size_t idx) const noexcept {
        // 0 is never a valid HPACK index — bail immediately.
        if (idx == 0) {
            return std::nullopt;
        }

        // First 61 slots are the static table, straight 1-based lookup.
        if (idx <= HPackStatic::STATIC_SIZE) {
            return HPackStatic::at(idx - 1);
        }

        // Anything past that lands in the dynamic table, offset back to its own
        // 0-based position.
        if (const auto FIELD = m_dyn.at_positon(idx - HPackStatic::STATIC_SIZE - 1);
            FIELD.has_value()) {
            return FIELD;
        }

        // Didn't land in either table — clean miss.
        return std::nullopt;
    }

    /**
     * @brief Same resolution as operator[], but throws instead of handing back an empty
     * optional — reach for this when a miss means the peer sent a busted index, not a
     * legitimate "not there".
     * @param idx the 1-based HPACK index to resolve.
     * @return the resolved header entry.
     * @throws std::out_of_range if `idx` doesn't resolve in either table.
     */
    [[nodiscard]] interfaces::io::HeaderEntry at(std::size_t idx) const {
        // Reuse operator[]'s resolution, just escalate a miss to a throw instead of
        // handing back an empty optional.
        if (auto field = (*this)[idx]) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
            return *field;
        }
        throw std::out_of_range{"hpack::HeaderTable: invalid index"};
    }


    /**
     * @brief Searches both tables for a name/value pair, static first then dynamic, full match
     * before name-only — the exact priority RFC 7541 wants so the encoder always picks the
     * cheapest representation available. Dynamic hits get their index shifted by the static
     * table size since HPACK indexing is unified, no separate address spaces here (that's
     * QPACK's whole deal).
     * @param name the header name to look for.
     * @param value the header value to look for.
     * @return a SearchResult describing the best match found (full match, name-only, or
     * nothing) and its unified index.
     */
    [[nodiscard]] shared_codec::SearchResult search(std::string_view name,
                                                    std::string_view value) const noexcept {
        // Cheapest win first — a full name+value match in the static table needs no
        // offsetting since static indices are already the unified ones.
        if (auto result =
                HPackStatic::search_full_match<shared_codec::IndexCalculation::H_PACK>(name, value);
            result.found()) {
            return result;
        }

        // Same full match, but in the dynamic table — its index has to get shifted past
        // the static table's 61 slots to land in HPACK's unified index space.
        if (auto result =
                m_dyn.search_full_match<shared_codec::IndexCalculation::H_PACK>(name, value);
            result.found()) {
            return shared_codec::SearchResult{result.index() + HPackStatic::STATIC_SIZE + 1, true,
                                              true};
        }

        // No full match anywhere — fall back to a name-only hit in the static table.
        if (auto result =
                HPackStatic::search_name_only<shared_codec::IndexCalculation::H_PACK>(name);
            result.found()) {
            return result;
        }

        // Same name-only fallback for the dynamic table, offset the same way as above.
        if (auto result = m_dyn.search_name_only<shared_codec::IndexCalculation::H_PACK>(name);
            result.found()) {
            return shared_codec::SearchResult{result.index() + HPackStatic::STATIC_SIZE + 1, true,
                                              false};
        }

        // Nothing in either table — encoder's gonna have to send it as a fresh literal.
        return shared_codec::SearchResult::none();
    }

    /**
     * @brief Inserts a name/value pair at the front of the dynamic table, evicting the oldest
     * entries first if it doesn't fit under the byte budget — straight RFC 7541 §4.4 eviction
     * motion, no cap.
     * @param name the header name to insert.
     * @param value the header value to insert.
     * @return the freshly-inserted entry's 0-based dynamic-table position (0 = most recent),
     * not yet offset into HPACK's unified index space — callers add
     * `HPackStatic::STATIC_SIZE + 1` themselves.
     */
    std::size_t insert(std::string_view name, std::string_view value) {
        return m_dyn.insert<shared_codec::IndexCalculation::H_PACK>(name, value);
    }

    /**
     * @brief Same insert as the string_view overload, but takes a well-known Token for the
     * name instead of a raw string — same eviction rules apply underneath.
     * @param token the well-known header name token to insert.
     * @param value the header value to insert.
     * @return the freshly-inserted entry's 0-based dynamic-table position (0 = most recent).
     */
    std::size_t insert(interfaces::io::types::Token token, std::string_view value) {
        return m_dyn.insert<shared_codec::IndexCalculation::H_PACK>(token, value);
    }

    /**
     * @brief Resizes the dynamic table's byte budget, evicting the oldest entries until usage
     * fits under the new cap — this is what a Dynamic Table Size Update instruction drives on
     * the wire.
     * @param new_max the new maximum size in bytes.
     */
    void set_max_size(std::size_t new_max) { m_dyn.set_max_size(new_max); }

    /**
     * @brief Gets the dynamic table's configured byte budget.
     * @return the max size in bytes.
     */
    [[nodiscard]] std::size_t max_size() const noexcept { return m_dyn.get_max_size(); }
    /**
     * @brief Gets how many bytes the dynamic table is actually holding right now.
     * @return the current size in bytes (sum of name + value + 32 overhead per live entry).
     */
    [[nodiscard]] std::size_t current_size() const noexcept { return m_dyn.get_current_size(); }
    /**
     * @brief Gets the number of entries currently alive in the dynamic table.
     * @return the dynamic table's live entry count.
     */
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.get_size(); }
    /**
     * @brief Gets the total addressable entry count across both tables combined — lowkey the
     * number every valid HPACK index has to fit under.
     * @return the static table size plus however many entries are currently alive in the
     * dynamic table.
     */
    [[nodiscard]] std::size_t total_entries() const noexcept {
        return HPackStatic::STATIC_SIZE + m_dyn.get_size();
    }

  private:
    shared_codec::table::DynamicTable m_dyn;
};

} // namespace io::codec::hpack
