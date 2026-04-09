export module qpack:table;

import std;
import transport_codec_shared;
import transport_shared;
import :types;

export namespace transport::codec::qpack {

inline const std::array<std::shared_ptr<shared::http::HeaderField<true>>, 99> k_static_table = {
    /* 0  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Authority, ""),
    /* 1  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Path, "/"),
    /* 2  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Age, "0"),
    /* 3  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentDisposition, ""),
    /* 4  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentLength, "0"),
    /* 5  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Cookie, ""),
    /* 6  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Date, ""),
    /* 7  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ETag, ""),
    /* 8  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfModifiedSince, ""),
    /* 9  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfNoneMatch, ""),
    /* 10 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::LastModified, ""),
    /* 11 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Link, ""),
    /* 12 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Location, ""),
    /* 13 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Referer, ""),
    /* 14 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::SetCookie, ""),
    /* 15 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "CONNECT"),
    /* 16 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "DELETE"),
    /* 17 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "GET"),
    /* 18 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "HEAD"),
    /* 19 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "OPTIONS"),
    /* 20 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "POST"),
    /* 21 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "PUT"),
    /* 22 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Scheme, "http"),
    /* 23 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Scheme, "https"),
    /* 24 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "103"),
    /* 25 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "200"),
    /* 26 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "304"),
    /* 27 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "404"),
    /* 28 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "503"),
    /* 29 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Accept, "*/*"),
    /* 30 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Accept, "application/dns-message"),
    /* 31 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptEncoding, "gzip, deflate, br"),
    /* 32 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptRanges, "bytes"),
    /* 33 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowHeaders, "cache-control"),
    /* 34 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowHeaders, "content-type"),
    /* 35 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowOrigin, "*"),
    /* 36 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, "max-age=0"),
    /* 37 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, "max-age=2592000"),
    /* 38 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, "max-age=604800"),
    /* 39 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, "no-cache"),
    /* 40 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, "no-store"),
    /* 41 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, "public, max-age=31536000"),
    /* 42 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentEncoding, "br"),
    /* 43 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentEncoding, "gzip"),
    /* 44 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "application/dns-message"),
    /* 45 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "application/javascript"),
    /* 46 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "application/json"),
    /* 47 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType,
                                                      "application/x-www-form-urlencoded"),
    /* 48 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "image/gif"),
    /* 49 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "image/jpeg"),
    /* 50 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "image/png"),
    /* 51 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "text/css"),
    /* 52 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "text/html; charset=utf-8"),
    /* 53 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "text/plain"),
    /* 54 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, "text/plain;charset=utf-8"),
    /* 55 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Range, "bytes=0-"),
    /* 56 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::StrictTransportSecurity, "max-age=31536000"),
    /* 57 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::StrictTransportSecurity,
                                                      "max-age=31536000; includesubdomains"),
    /* 58 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::StrictTransportSecurity,
                                                      "max-age=31536000; includesubdomains; preload"),
    /* 59 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Vary, "accept-encoding"),
    /* 60 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Vary, "origin"),
    /* 61 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::XContentTypeOptions, "nosniff"),
    /* 62 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::XXssProtection, "1; mode=block"),
    /* 63 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "100"),
    /* 64 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "204"),
    /* 65 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "206"),
    /* 66 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "302"),
    /* 67 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "400"),
    /* 68 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "403"),
    /* 69 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "421"),
    /* 70 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "425"),
    /* 71 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "500"),
    /* 72 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptLanguage, ""),
    /* 73 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowCredentials, "FALSE"),
    /* 74 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowCredentials, "TRUE"),
    /* 75 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowHeaders, "*"),
    /* 76 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowMethods, "get"),
    /* 77 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowMethods,
                                                      "get, post, options"),
    /* 78 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowMethods, "options"),
    /* 79 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlExposeHeaders,
                                                      "content-length"),
    /* 80 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlRequestHeaders, "content-type"),
    /* 81 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlRequestMethod, "get"),
    /* 82 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlRequestMethod, "post"),
    /* 83 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AltSvc, "clear"),
    /* 84 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Authorization, ""),
    /* 85 */
    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentSecurityPolicy,
                                                      "script-src 'none'; object-src 'none'; base-uri 'none'"),
    /* 86 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::EarlyData, "1"),
    /* 87 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ExpectCt, ""),
    /* 88 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Forwarded, ""),
    /* 89 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfRange, ""),
    /* 90 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Origin, ""),
    /* 91 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Purpose, "prefetch"),
    /* 92 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Server, ""),
    /* 93 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::TimingAllowOrigin, "*"),
    /* 94 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::UpgradeInsecureRequests, "1"),
    /* 95 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::UserAgent, ""),
    /* 96 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::XForwardedFor, ""),
    /* 97 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::XFrameOptions, "deny"),
    /* 98 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::XFrameOptions, "sameorigin"),
};

using QPackStatic = shared_codec::table::StaticTable<k_static_table>;

// HeaderTable — RFC 9204 separate index spaces
class QPackTable {
  public:
    explicit QPackTable(std::size_t max_capacity = 0) : m_dyn{max_capacity} {}

    template <bool IsIndexPostBase = false, bool IsStatic>
    std::optional<std::shared_ptr<shared::http::HeaderField<IsStatic>>>
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
                if (idx > base)
                    return std::nullopt;
                abs = base - idx;
            }

            if (abs >= m_dyn.get_size())
                return std::nullopt;

            // Your DynamicTable uses 1-based "generations"
            return m_dyn.at_generation(abs + 1);
        }
    }


    template <bool IsIndexPostBase = false, bool IsStatic>
    std::shared_ptr<shared::http::HeaderField<true>> at(std::size_t idx, std::size_t base = 0) const {
        if (auto r = this->operator[]<IsIndexPostBase, IsStatic>(idx, base))
            return *r;
        throw std::out_of_range{"qpack::HeaderTable: invalid index"};
    }

    shared_codec::SearchResult search(std::string_view name, std::string_view value) const noexcept {
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

    std::size_t encode_ric(std::size_t ric) const noexcept {
        if (ric == 0)
            return 0;

        const std::size_t MaxEntries = m_dyn.get_max_size() / 32;
        return (ric % (2 * MaxEntries)) + 1;
    }

    std::size_t decode_ric(std::size_t enc_ric) const noexcept {
        if (enc_ric == 0)
            return 0;

        const std::size_t MaxEntries = m_dyn.get_max_size() / 32;
        const std::size_t FullRange = 2 * MaxEntries;

        // Total number of dynamic table inserts known to the decoder
        const std::size_t TotalInst = m_dyn.get_insert_count();

        // Use the RFC algorithm to find the closest RIC to our current count
        std::size_t max_ric = TotalInst + MaxEntries;
        std::size_t ric = (max_ric / FullRange) * FullRange + (enc_ric - 1);

        if (ric > max_ric && ric >= FullRange) {
            ric -= FullRange;
        }

        return ric;
    }

    bool is_ready(std::size_t ric) const noexcept { return ric <= m_dyn.get_insert_count(); }

    std::size_t insert(std::shared_ptr<shared::http::HeaderField<true>> field) {
        const std::size_t gen = m_dyn.insert(std::move(field));
        return gen == 0 ? shared_codec::SIZE_MAX : gen - 1;
    }

    std::size_t insert(std::string_view name, std::string_view value) {
        const std::size_t gen = m_dyn.insert(name, value);
        return gen == 0 ? shared_codec::SIZE_MAX : gen - 1;
    }

    void set_max_size(std::size_t cap) { m_dyn.set_max_size(cap); }

    std::size_t insert_count() const noexcept { return m_dyn.get_insert_count(); }
    std::size_t used() const noexcept { return m_dyn.get_current_size(); }
    std::size_t dynamic_count() const noexcept { return m_dyn.get_size(); }
    std::size_t max_size() const noexcept { return m_dyn.get_max_size(); }

  private:
    [[nodiscard]] std::size_t abs_to_rel(std::size_t abs) const noexcept {
        const std::size_t ic = m_dyn.get_insert_count();
        if (abs >= ic)
            return shared_codec::SIZE_MAX;
        return ic - 1 - abs;
    }

    [[nodiscard]] std::size_t rel_to_abs(std::size_t rel) const noexcept {
        const std::size_t ic = m_dyn.get_insert_count();
        if (ic == 0 || rel >= ic)
            return shared_codec::SIZE_MAX;
        return ic - 1 - rel;
    }

    // Helper for post-base indexing:
    // post-base: abs = base + 1 + pb
    [[nodiscard]] static std::size_t post_base_to_absolut_index(std::size_t base, std::size_t pb) noexcept {
        return base + 1 + pb;
    }

    // [[nodiscard]] static std::size_t abs_to_post_base(std::size_t base, std::size_t abs) noexcept {
    //     if (abs <= base)
    //         return shared_codec::SIZE_MAX;
    //     return abs - base - 1;
    // }

    shared_codec::table::DynamicTable m_dyn;
};

} // namespace transport::codec::qpack
