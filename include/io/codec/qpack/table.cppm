export module io_codec_qpack:table;

import std;
import io_codec_shared;
import interfaces;
import :types;

export namespace io::codec::qpack {

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
    explicit QPackTable(std::size_t max_capacity = 0) : m_dyn{max_capacity} {}

    template <bool IsIndexPostBase = false, bool IsStatic>
    std::optional<std::shared_ptr<interfaces::io::HeaderField<IsStatic>>>
    operator[](std::size_t idx, std::size_t base = 0) const noexcept {
        if constexpr (IsStatic) {
            return QPackStatic::at(idx);
        } else {
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

            if (abs >= m_dyn.get_size()) {
                return std::nullopt;
            }

            // Your DynamicTable uses 1-based "generations"
            return m_dyn.at_generation(abs + 1);
        }
    }


    template <bool IsIndexPostBase = false, bool IsStatic>
    [[nodiscard]] std::shared_ptr<interfaces::io::HeaderField<true>> at(std::size_t idx,
                                                                      std::size_t base = 0) const {
        if (auto found = this->operator[]<IsIndexPostBase, IsStatic>(idx, base)) {
            return *found;
        }
        throw std::out_of_range{"qpack::HeaderTable: invalid index"};
    }

    [[nodiscard]] shared_codec::SearchResult search(std::string_view name,
                                                    std::string_view value) const noexcept {
        if (auto result = QPackStatic::search_full_match(name, value); result.found()) {
            return result;
        }

        if (auto result = m_dyn.search_full_match(name, value); result.found()) {
            return result;
        }

        if (auto result = QPackStatic::search_name_only(name); result.found()) {
            return result;
        }

        if (auto result = m_dyn.search_name_only(name); result.found()) {
            return result;
        }

        return shared_codec::SearchResult::none();
    }

    [[nodiscard]] std::size_t encode_ric(std::size_t ric) const noexcept {
        if (ric == 0) {
            return 0;
        }

        const std::size_t MAX_ENTRIES = m_dyn.get_max_size() / 32;
        return (ric % (2 * MAX_ENTRIES)) + 1;
    }

    [[nodiscard]] std::size_t decode_ric(std::size_t enc_ric) const noexcept {
        if (enc_ric == 0) {
            return 0;
        }

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

    [[nodiscard]] bool is_ready(std::size_t ric) const noexcept {
        return ric <= m_dyn.get_insert_count();
    }

    std::size_t insert(std::shared_ptr<interfaces::io::HeaderField<true>> field) {
        const std::size_t GEN = m_dyn.insert(std::move(field));
        return GEN == 0 ? shared_codec::SIZE_MAX : GEN - 1;
    }

    std::size_t insert(std::string_view name, std::string_view value) {
        const std::size_t GEN = m_dyn.insert(name, value);
        return GEN == 0 ? shared_codec::SIZE_MAX : GEN - 1;
    }

    void set_max_size(std::size_t cap) { m_dyn.set_max_size(cap); }

    [[nodiscard]] std::size_t insert_count() const noexcept { return m_dyn.get_insert_count(); }
    [[nodiscard]] std::size_t used() const noexcept { return m_dyn.get_current_size(); }
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.get_size(); }
    [[nodiscard]] std::size_t max_size() const noexcept { return m_dyn.get_max_size(); }

  private:
    [[nodiscard]] std::size_t abs_to_rel(std::size_t abs) const noexcept {
        const std::size_t IC = m_dyn.get_insert_count();
        if (abs >= IC) {
            return shared_codec::SIZE_MAX;
        }
        return IC - 1 - abs;
    }

    [[nodiscard]] std::size_t rel_to_abs(std::size_t rel) const noexcept {
        const std::size_t INC = m_dyn.get_insert_count();
        if (INC == 0 || rel >= INC) {
            return shared_codec::SIZE_MAX;
        }
        return INC - 1 - rel;
    }

    // Helper for post-base indexing:
    // post-base: abs = base + 1 + pb
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
