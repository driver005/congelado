export module io_codec_hpack:table;

import std;
import io_codec_shared;
import interfaces;
import :consts;

export namespace io::codec::hpack {

// TODO: make length a constant

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
    explicit HPackTable(std::size_t max_size = DEFAULT_MAX_TABLE_SIZE) : m_dyn{max_size} {}

    [[nodiscard]] std::optional<interfaces::io::HeaderEntry>
    operator[](std::size_t idx) const noexcept {
        if (idx == 0) {
            return std::nullopt;
        }

        if (idx <= HPackStatic::STATIC_SIZE) {
            return HPackStatic::at(idx - 1);
        }

        if (const auto FIELD = m_dyn.at_positon(idx - HPackStatic::STATIC_SIZE - 1);
            FIELD.has_value()) {
            return FIELD;
        }

        return std::nullopt;
    }

    [[nodiscard]] interfaces::io::HeaderEntry at(std::size_t idx) const {
        if (auto field = (*this)[idx]) {
            return *field;
        }
        throw std::out_of_range{"hpack::HeaderTable: invalid index"};
    }


    [[nodiscard]] shared_codec::SearchResult search(std::string_view name,
                                                    std::string_view value) const noexcept {
        if (auto result =
                HPackStatic::search_full_match<shared_codec::IndexCalculation::H_PACK>(name, value);
            result.found()) {
            return result;
        }

        if (auto result =
                m_dyn.search_full_match<shared_codec::IndexCalculation::H_PACK>(name, value);
            result.found()) {
            return shared_codec::SearchResult{result.index() + HPackStatic::STATIC_SIZE + 1, true,
                                              true};
        }

        if (auto result =
                HPackStatic::search_name_only<shared_codec::IndexCalculation::H_PACK>(name);
            result.found()) {
            return result;
        }

        if (auto result = m_dyn.search_name_only<shared_codec::IndexCalculation::H_PACK>(name);
            result.found()) {
            return shared_codec::SearchResult{result.index() + HPackStatic::STATIC_SIZE + 1, true,
                                              false};
        }

        return shared_codec::SearchResult::none();
    }

    std::size_t insert(std::string_view name, std::string_view value) {
        return m_dyn.insert<shared_codec::IndexCalculation::H_PACK>(name, value);
    }

    std::size_t insert(interfaces::io::types::Token token, std::string_view value) {
        return m_dyn.insert<shared_codec::IndexCalculation::H_PACK>(token, value);
    }

    void set_max_size(std::size_t new_max) { m_dyn.set_max_size(new_max); }

    [[nodiscard]] std::size_t max_size() const noexcept { return m_dyn.get_max_size(); }
    [[nodiscard]] std::size_t current_size() const noexcept { return m_dyn.get_current_size(); }
    [[nodiscard]] std::size_t dynamic_count() const noexcept { return m_dyn.get_size(); }
    [[nodiscard]] std::size_t total_entries() const noexcept {
        return HPackStatic::STATIC_SIZE + m_dyn.get_size();
    }

  private:
    shared_codec::table::DynamicTable m_dyn;
};

} // namespace io::codec::hpack
