export module io_codec_qpack:table;

import std;
import io_codec_shared;
import interfaces;
import :types;

export namespace io::codec::qpack {

// FIXME(clang-tidy): bugprone-throwing-static-initialization — STATIC_TABLE's shared_ptr/
// make_shared init can throw (bad_alloc), and it's used below as a non-type template argument
// to shared_codec::table::StaticTable<STATIC_TABLE>, so it can't be turned into a lazily
// initialized function-local static without a wider API change across this module.
inline const std::array<std::shared_ptr<interfaces::io::HeaderField<true>>, 99> STATIC_TABLE = {
    /* 0  */ std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::AUTHORITY, ""),
    /* 1  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::PATH, "/"),
    /* 2  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::AGE, "0"),
    /* 3  */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_DISPOSITION, ""),
    /* 4  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_LENGTH,
                                                      "0"),
    /* 5  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::COOKIE, ""),
    /* 6  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::DATE, ""),
    /* 7  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::E_TAG, ""),
    /* 8  */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::IF_MODIFIED_SINCE, ""),
    /* 9  */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::IF_NONE_MATCH,
                                                      ""),
    /* 10 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::LAST_MODIFIED,
                                                      ""),
    /* 11 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::LINK, ""),
    /* 12 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::LOCATION, ""),
    /* 13 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::REFERER, ""),
    /* 14 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SET_COOKIE, ""),
    /* 15 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD,
                                                      "CONNECT"),
    /* 16 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD,
                                                      "DELETE"),
    /* 17 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD, "GET"),
    /* 18 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD, "HEAD"),
    /* 19 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD,
                                                      "OPTIONS"),
    /* 20 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD, "POST"),
    /* 21 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::METHOD, "PUT"),
    /* 22 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SCHEME, "http"),
    /* 23 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SCHEME,
                                                      "https"),
    /* 24 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "103"),
    /* 25 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "200"),
    /* 26 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "304"),
    /* 27 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "404"),
    /* 28 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "503"),
    /* 29 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT, "*/*"),
    /* 30 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT,
                                                      "application/dns-message"),
    /* 31 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT_ENCODING,
                                                      "gzip, deflate, br"),
    /* 32 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT_RANGES,
                                                      "bytes"),
    /* 33 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_HEADERS, "cache-control"),
    /* 34 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_HEADERS, "content-type"),
    /* 35 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_ORIGIN, "*"),
    /* 36 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                      "max-age=0"),
    /* 37 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                      "max-age=2592000"),
    /* 38 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                      "max-age=604800"),
    /* 39 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                      "no-cache"),
    /* 40 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                      "no-store"),
    /* 41 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CACHE_CONTROL,
                                                      "public, max-age=31536000"),
    /* 42 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_ENCODING, "br"),
    /* 43 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_ENCODING, "gzip"),
    /* 44 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "application/dns-message"),
    /* 45 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "application/javascript"),
    /* 46 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "application/json"),
    /* 47 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "application/x-www-form-urlencoded"),
    /* 48 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "image/gif"),
    /* 49 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "image/jpeg"),
    /* 50 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "image/png"),
    /* 51 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "text/css"),
    /* 52 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "text/html; charset=utf-8"),
    /* 53 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "text/plain"),
    /* 54 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::CONTENT_TYPE,
                                                      "text/plain;charset=utf-8"),
    /* 55 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::RANGE,
                                                      "bytes=0-"),
    /* 56 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::STRICT_TRANSPORT_SECURITY, "max-age=31536000"),
    /* 57 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::STRICT_TRANSPORT_SECURITY,
        "max-age=31536000; includesubdomains"),
    /* 58 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::STRICT_TRANSPORT_SECURITY,
        "max-age=31536000; includesubdomains; preload"),
    /* 59 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::VARY,
                                                      "accept-encoding"),
    /* 60 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::VARY, "origin"),
    /* 61 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::X_CONTENT_TYPE_OPTIONS, "nosniff"),
    /* 62 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::X_XSS_PROTECTION, "1; mode=block"),
    /* 63 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "100"),
    /* 64 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "204"),
    /* 65 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "206"),
    /* 66 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "302"),
    /* 67 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "400"),
    /* 68 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "403"),
    /* 69 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "421"),
    /* 70 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "425"),
    /* 71 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::STATUS, "500"),
    /* 72 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ACCEPT_LANGUAGE,
                                                      ""),
    /* 73 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_CREDENTIALS, "FALSE"),
    /* 74 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_CREDENTIALS, "TRUE"),
    /* 75 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_HEADERS, "*"),
    /* 76 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_METHODS, "get"),
    /* 77 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_METHODS, "get, post, options"),
    /* 78 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_ALLOW_METHODS, "options"),
    /* 79 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_EXPOSE_HEADERS, "content-length"),
    /* 80 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_REQUEST_HEADERS, "content-type"),
    /* 81 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_REQUEST_METHOD, "get"),
    /* 82 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::ACCESS_CONTROL_REQUEST_METHOD, "post"),
    /* 83 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ALT_SVC,
                                                      "clear"),
    /* 84 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::AUTHORIZATION,
                                                      ""),
    /* 85 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::CONTENT_SECURITY_POLICY,
        "script-src 'none'; object-src 'none'; base-uri 'none'"),
    /* 86 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::EARLY_DATA,
                                                      "1"),
    /* 87 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::EXPECT_CT, ""),
    /* 88 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::FORWARDED, ""),
    /* 89 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::IF_RANGE, ""),
    /* 90 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::ORIGIN, ""),
    /* 91 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::PURPOSE,
                                                      "prefetch"),
    /* 92 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::SERVER, ""),
    /* 93 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::TIMING_ALLOW_ORIGIN, "*"),
    /* 94 */
    std::make_shared<interfaces::io::HeaderField<true>>(
        interfaces::io::types::Token::UPGRADE_INSECURE_REQUESTS, "1"),
    /* 95 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::USER_AGENT, ""),
    /* 96 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::X_FORWARDED_FOR,
                                                      ""),
    /* 97 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::X_FRAME_OPTIONS,
                                                      "deny"),
    /* 98 */
    std::make_shared<interfaces::io::HeaderField<true>>(interfaces::io::types::Token::X_FRAME_OPTIONS,
                                                      "sameorigin"),
};

using QPackStatic = shared_codec::table::StaticTable<STATIC_TABLE>;

// HeaderTable — RFC 9204 separate index spaces
class QPackTable {
  public:
    /**
     * @brief Spins up a QPACK header table — the RFC 9204 static table (99 entries) is free,
     * the dynamic table starts empty with `max_capacity` bytes of headroom.
     * @param max_capacity the dynamic table's byte budget, defaults to 0 (no dynamic entries
     * allowed until set_max_size() bumps it up).
     */
    explicit QPackTable(std::size_t max_capacity = 0) : m_dyn{max_capacity} {}

    /**
     * @brief Resolves an index into its header field, in whichever of QPACK's separate address
     * spaces the template flags select — static indexing is direct, dynamic indexing needs
     * `base` to translate relative or post-base indices into an absolute table slot per
     * RFC 9204 §3.2. Straight footgun territory if `IsStatic`/`IsIndexPostBase` don't match
     * how the wire byte was actually encoded, so get those bits right upstream.
     * @tparam IsIndexPostBase when true, `idx` is a post-base index (absolute = base + 1 +
     * idx); when false, it's a relative index counted backward from `base`.
     * @tparam IsStatic when true, `idx` addresses the static table directly and `base` is
     * ignored; when false, it addresses the dynamic table.
     * @param idx the index to resolve, interpreted per `IsStatic`/`IsIndexPostBase`.
     * @param base the field section's Base value (RFC 9204 §4.5.1), only used for dynamic
     * lookups.
     * @return the resolved header field, or std::nullopt if the index doesn't land on a live
     * entry.
     */
    template <bool IsIndexPostBase = false, bool IsStatic>
    std::optional<std::shared_ptr<interfaces::io::HeaderField<IsStatic>>>
    operator[](std::size_t idx, std::size_t base = 0) const noexcept {
        // Static lookups are direct, no Base math involved.
        if constexpr (IsStatic) {
            return QPackStatic::at(idx);
        } else {
            // Dynamic — translate whichever index flavor the wire byte encoded into an
            // absolute table position.
            std::size_t abs = 0;
            if constexpr (IsIndexPostBase) {
                // Absolute = Base + 1 + Post-Base Index
                abs = base + 1 + idx;
            } else {
                // Absolute = Base - Relative Index
                if (idx > base) {
                    return std::nullopt;
                }
                abs = base - idx;
            }

            // Past the live entries — nothing there (evicted or never existed).
            if (abs >= m_dyn.get_size()) {
                return std::nullopt;
            }

            // Your DynamicTable uses 1-based "generations"
            return m_dyn.at_generation(abs + 1);
        }
    }


    /**
     * @brief Same resolution as operator[], but throws instead of returning std::nullopt — use
     * this when a miss means the peer's wire data is straight cooked, not a legit "not found".
     * @tparam IsIndexPostBase when true, `idx` is a post-base index; when false, a relative one.
     * @tparam IsStatic when true, `idx` addresses the static table directly.
     * @param idx the index to resolve, interpreted per `IsStatic`/`IsIndexPostBase`.
     * @param base the field section's Base value, only used for dynamic lookups.
     * @return the resolved header field.
     * @throws std::out_of_range if the index doesn't resolve to a live entry.
     */
    template <bool IsIndexPostBase = false, bool IsStatic>
    [[nodiscard]] std::shared_ptr<interfaces::io::HeaderField<true>> at(std::size_t idx,
                                                                      std::size_t base = 0) const {
        // Same resolution as operator[], just throws instead of returning empty on a miss.
        if (auto found = this->operator[]<IsIndexPostBase, IsStatic>(idx, base)) {
            return *found;
        }
        throw std::out_of_range{"qpack::HeaderTable: invalid index"};
    }

    /**
     * @brief Searches both tables for a name/value pair, static first then dynamic, full match
     * before name-only — same priority ordering as HPACK so the encoder always reaches for the
     * cheapest representation available. Unlike HPACK, QPACK keeps separate index spaces, so no
     * offset shuffling between static and dynamic hits here.
     * @param name the header name to look for.
     * @param value the header value to look for.
     * @return a SearchResult describing the best match found and its index within whichever
     * table it landed in.
     */
    [[nodiscard]] shared_codec::SearchResult search(std::string_view name,
                                                    std::string_view value) const noexcept {
        // Static full match first — cheapest possible representation.
        if (auto result = QPackStatic::search_full_match(name, value); result.found()) {
            return result;
        }

        // Dynamic full match next — no offsetting needed since QPACK keeps the static
        // and dynamic index spaces separate, unlike HPACK's unified space.
        if (auto result = m_dyn.search_full_match(name, value); result.found()) {
            return result;
        }

        // No full match — fall back to a name-only hit, static table first.
        if (auto result = QPackStatic::search_name_only(name); result.found()) {
            return result;
        }

        // Then dynamic name-only.
        if (auto result = m_dyn.search_name_only(name); result.found()) {
            return result;
        }

        // Complete miss — nothing to reference, gotta go full literal.
        return shared_codec::SearchResult::none();
    }

    /**
     * @brief Wire-encodes a Required Insert Count per RFC 9204 §4.5.1.2 — mods it into the
     * range the field section prefix's variable-length integer can carry, so the decoder can
     * unambiguously reconstruct it later via decode_ric(). This is the compression trick that
     * keeps RIC small on the wire instead of shipping a raw ever-growing counter.
     * @param ric the true (unencoded) Required Insert Count; 0 means no dynamic table
     * dependency and encodes to 0 with no further math.
     * @return the wire-format encoded RIC.
     */
    [[nodiscard]] std::size_t encode_ric(std::size_t ric) const noexcept {
        // Zero is its own special case — no dynamic table dependency, no encoding needed.
        if (ric == 0) {
            return 0;
        }

        // Wrap into the range the field section prefix's varint can carry.
        const std::size_t MAX_ENTRIES = m_dyn.get_max_size() / 32;
        return (ric % (2 * MAX_ENTRIES)) + 1;
    }

    /**
     * @brief Reverses encode_ric() — reconstructs the true Required Insert Count from its
     * wire-encoded form using the RFC 9204 §4.5.1.2 algorithm, picking whichever candidate is
     * closest to the decoder's current known insert count. Get this wrong and you'll either
     * block on entries that already arrived or read a dynamic-table slot that's been evicted —
     * both are straight L's for correctness.
     * @param enc_ric the wire-format encoded RIC from the field section prefix.
     * @return the true (unencoded) Required Insert Count; 0 if `enc_ric` was 0.
     */
    [[nodiscard]] std::size_t decode_ric(std::size_t enc_ric) const noexcept {
        // Encoded 0 always means "no dependency" — nothing to reconstruct.
        if (enc_ric == 0) {
            return 0;
        }

        // Same wrap window encode_ric() folded the true RIC into.
        const std::size_t MAX_ENTRIES = m_dyn.get_max_size() / 32;
        const std::size_t FULL_RANGE = 2 * MAX_ENTRIES;

        // Total number of dynamic table inserts known to the decoder
        const std::size_t TOTAL_INST = m_dyn.get_insert_count();

        // Use the RFC algorithm to find the closest RIC to our current count
        std::size_t max_ric = TOTAL_INST + MAX_ENTRIES;
        std::size_t ric = ((max_ric / FULL_RANGE) * FULL_RANGE) + (enc_ric - 1);

        if (ric > max_ric && ric >= FULL_RANGE) {
            ric -= FULL_RANGE;
        }

        return ric;
    }

    /**
     * @brief Checks whether the decoder has actually received enough dynamic table inserts to
     * safely process a field section that depends on a given Required Insert Count — this is
     * QPACK's blocking-stream guard from RFC 9204 §2.1.2.
     * @param ric the Required Insert Count a field section is waiting on.
     * @return true if the decoder's insert count has caught up (or exceeds) `ric`, false if the
     * stream should still be treated as blocked.
     */
    [[nodiscard]] bool is_ready(std::size_t ric) const noexcept {
        return ric <= m_dyn.get_insert_count();
    }

    /**
     * @brief Inserts an already-built header field at the front of the dynamic table, evicting
     * oldest entries first if it doesn't fit — used for duplicate/insert-with-indexed-name
     * encoder stream instructions where the field already exists somewhere.
     * @param field the header field to insert, moved in.
     * @return the freshly-inserted entry's absolute index, or shared_codec::SIZE_MAX if the
     * table couldn't fit it (entry got evicted immediately after insertion, generation 0).
     */
    std::size_t insert(std::shared_ptr<interfaces::io::HeaderField<true>> field) {
        // Insert, then translate the 1-based generation the table hands back into a
        // 0-based absolute index — generation 0 means it got evicted on arrival.
        const std::size_t GEN = m_dyn.insert(std::move(field));
        return GEN == 0 ? shared_codec::SIZE_MAX : GEN - 1;
    }

    /**
     * @brief Inserts a fresh name/value pair at the front of the dynamic table, evicting oldest
     * entries first if it doesn't fit under the byte budget.
     * @param name the header name to insert.
     * @param value the header value to insert.
     * @return the freshly-inserted entry's absolute index, or shared_codec::SIZE_MAX if it
     * didn't fit (evicted immediately, generation 0).
     */
    std::size_t insert(std::string_view name, std::string_view value) {
        // Same generation-to-index translation as the field overload above.
        const std::size_t GEN = m_dyn.insert(name, value);
        return GEN == 0 ? shared_codec::SIZE_MAX : GEN - 1;
    }

    /**
     * @brief Resizes the dynamic table's byte budget, evicting oldest entries until usage fits
     * under the new cap — driven by Set Dynamic Table Capacity encoder stream instructions.
     * @param cap the new maximum size in bytes.
     */
    void set_max_size(std::size_t cap) { m_dyn.set_max_size(cap); }

    /**
     * @brief Gets the total number of entries ever inserted into the dynamic table, evicted or
     * not — this is the raw counter RIC math is built on top of.
     * @return the dynamic table's cumulative insert count.
     */
    [[nodiscard]] std::size_t insert_count() const noexcept { return m_dyn.get_insert_count(); }
    /**
     * @brief Gets how many bytes the dynamic table is actually holding right now.
     * @return the current size in bytes.
     */
    [[nodiscard]] std::size_t used() const noexcept { return m_dyn.get_current_size(); }
    /**
     * @brief Gets the number of entries currently alive in the dynamic table.
     * @return the dynamic table's live entry count.
     */
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.get_size(); }
    /**
     * @brief Gets the dynamic table's configured byte budget.
     * @return the max size in bytes.
     */
    [[nodiscard]] std::size_t max_size() const noexcept { return m_dyn.get_max_size(); }

  private:
    /**
     * @brief Converts an absolute dynamic table index into a relative index, counted backward
     * from the table's newest entry — the direction RFC 9204 relative indexing runs.
     * @param abs the absolute index to convert.
     * @return the equivalent relative index, or shared_codec::SIZE_MAX if `abs` is at or past
     * the current insert count (i.e. doesn't exist yet).
     */
    [[nodiscard]] std::size_t abs_to_rel(std::size_t abs) const noexcept {
        // Anything at or past the current insert count doesn't exist yet.
        const std::size_t IC = m_dyn.get_insert_count();
        if (abs >= IC) {
            return shared_codec::SIZE_MAX;
        }
        return IC - 1 - abs;
    }

    /**
     * @brief Converts a relative dynamic table index back into an absolute one — the inverse
     * of abs_to_rel().
     * @param rel the relative index to convert.
     * @return the equivalent absolute index, or shared_codec::SIZE_MAX if the table's empty or
     * `rel` points past the oldest known entry.
     */
    [[nodiscard]] std::size_t rel_to_abs(std::size_t rel) const noexcept {
        // Empty table or an out-of-range relative index — nothing to resolve.
        const std::size_t INC = m_dyn.get_insert_count();
        if (INC == 0 || rel >= INC) {
            return shared_codec::SIZE_MAX;
        }
        return INC - 1 - rel;
    }

    // Helper for post-base indexing:
    // post-base: abs = base + 1 + pb
    /**
     * @brief Converts a post-base index into an absolute dynamic table index per RFC 9204
     * §3.2.6 — post-base entries are ones inserted after the field section's Base was fixed, so
     * they count forward from `base` instead of backward.
     * @param base the field section's Base value.
     * @param post_base the post-base index to convert.
     * @return the equivalent absolute index.
     */
    [[nodiscard]] static std::size_t post_base_to_absolut_index(std::size_t base,
                                                                std::size_t post_base) noexcept {
        return base + 1 + post_base;
    }

    // [[nodiscard]] static std::size_t abs_to_post_base(std::size_t base, std::size_t abs) noexcept
    // {
    //     if (abs <= base)
    //         return shared_codec::SIZE_MAX;
    //     return abs - base - 1;
    // }

    shared_codec::table::DynamicTable m_dyn;
};

} // namespace io::codec::qpack
