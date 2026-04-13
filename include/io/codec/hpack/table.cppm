export module io_codec_hpack:table;

import std;
import io_codec_shared;
import io_shared;
import :consts;

export namespace transport::codec::hpack {

inline const std::array<std::shared_ptr<shared::http::HeaderField<true>>, 61> k_static_table = {
    /* 0  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Authority, ""),
    /* 1  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "GET"),
    /* 2  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Method, "POST"),
    /* 3  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Path, "/"),
    /* 4  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Path, "/index.html"),
    /* 5  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Scheme, "http"),
    /* 6  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Scheme, "https"),
    /* 7  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "200"),
    /* 8  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "204"),
    /* 9  */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "206"),
    /* 10 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "304"),
    /* 11 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "400"),
    /* 12 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "404"),
    /* 13 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Status, "500"),
    /* 14 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptCharset, ""),
    /* 15 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptEncoding, "gzip, deflate"),
    /* 16 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptLanguage, ""),
    /* 17 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AcceptRanges, ""),
    /* 18 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Accept, ""),
    /* 19 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::AccessControlAllowOrigin, ""),
    /* 20 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Age, ""),
    /* 21 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Allow, ""),
    /* 22 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Authorization, ""),
    /* 23 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::CacheControl, ""),
    /* 24 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentDisposition, ""),
    /* 25 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentEncoding, ""),
    /* 26 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentLanguage, ""),
    /* 27 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentLength, ""),
    /* 28 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentLocation, ""),
    /* 29 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentRange, ""),
    /* 30 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ContentType, ""),
    /* 31 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Cookie, ""),
    /* 32 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Date, ""),
    /* 33 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ETag, ""),
    /* 34 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Expect, ""),
    /* 35 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Expires, ""),
    /* 36 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::From, ""),
    /* 37 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Host, ""),
    /* 38 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfMatch, ""),
    /* 39 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfModifiedSince, ""),
    /* 40 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfNoneMatch, ""),
    /* 41 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfRange, ""),
    /* 42 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::IfUnmodifiedSince, ""),
    /* 43 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::LastModified, ""),
    /* 44 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Link, ""),
    /* 45 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Location, ""),
    /* 46 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::MaxForwards, ""),
    /* 47 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ProxyAuthenticate, ""),
    /* 48 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::ProxyAuthorization, ""),
    /* 49 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Range, ""),
    /* 50 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Referer, ""),
    /* 51 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Refresh, ""),
    /* 52 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::RetryAfter, ""),
    /* 53 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Server, ""),
    /* 54 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::SetCookie, ""),
    /* 55 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::StrictTransportSecurity, ""),
    /* 56 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::TransferEncoding, ""),
    /* 57 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::UserAgent, ""),
    /* 58 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Vary, ""),
    /* 59 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::Via, ""),
    /* 60 */ std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::WwwAuthenticate, ""),
};

using HPackStatic = shared_codec::table::StaticTable<k_static_table>;

// HeaderTable — RFC 7541 unified index space
class HPackTable {
  public:
    explicit HPackTable(std::size_t max_size = DEFAULT_MAX_TABLE_SIZE) : m_dyn{max_size} {}

    [[nodiscard]] std::optional<shared::http::HeaderEntry> operator[](std::size_t idx) const noexcept {
        if (idx == 0)
            return std::nullopt;

        if (idx <= HPackStatic::STATIC_SIZE)
            return HPackStatic::at(idx - 1);

        if (const auto field = m_dyn.at_positon(idx - HPackStatic::STATIC_SIZE - 1); field.has_value())
            return field;

        return std::nullopt;
    }

    [[nodiscard]] shared::http::HeaderEntry at(std::size_t idx) const {
        if (auto r = (*this)[idx])
            return *r;
        throw std::out_of_range{"hpack::HeaderTable: invalid index"};
    }


    [[nodiscard]] shared_codec::SearchResult search(std::string_view name, std::string_view value) const noexcept {
        if (auto result = HPackStatic::search_full_match<shared_codec::IndexCalculation::HPack>(name, value);
            result.found()) {
            return result;
        }

        if (auto result = m_dyn.search_full_match<shared_codec::IndexCalculation::HPack>(name, value); result.found()) {
            return result;
        }

        if (auto result = HPackStatic::search_name_only<shared_codec::IndexCalculation::HPack>(name); result.found()) {
            return result;
        }

        if (auto result = m_dyn.search_name_only<shared_codec::IndexCalculation::HPack>(name); result.found()) {
            return result;
        }

        return shared_codec::SearchResult::none();
    }

    std::size_t insert(std::string_view name, std::string_view value) {
        return m_dyn.insert<shared_codec::IndexCalculation::HPack>(name, value);
    }

    std::size_t insert(shared::http::Token token, std::string_view value) {
        return m_dyn.insert<shared_codec::IndexCalculation::HPack>(std::move(token), value);
    }

    void set_max_size(std::size_t new_max) { m_dyn.set_max_size(new_max); }

    [[nodiscard]] std::size_t max_size() const noexcept { return m_dyn.get_max_size(); }
    [[nodiscard]] std::size_t current_size() const noexcept { return m_dyn.get_current_size(); }
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.get_size(); }
    [[nodiscard]] std::size_t total_entries() const noexcept { return HPackStatic::STATIC_SIZE + m_dyn.get_size(); }

  private:
    shared_codec::table::DynamicTable m_dyn;
};

} // namespace transport::codec::hpack
