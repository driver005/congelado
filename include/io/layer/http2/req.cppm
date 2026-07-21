module;
#include <cassert>
#include <ranges>
export module io_layer_http2:request;

import std;
import hashmap;
import interfaces;
import io_layer_shared;
import io_codec_hpack;
import utils_buffering;
import utils_encode;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpRequest : public interfaces::io::IRequest {
  public:
    /**
     * @brief Builds an HTTP/2 request for the given stream, static-header array starts fully
     * empty (all null shared_ptrs).
     * @param stream_id the stream this request rides on, forwarded straight to `IRequest`.
     */
    explicit HttpRequest(std::uint32_t stream_id)
        : interfaces::io::IRequest{stream_id}, m_static_headers{} {}

    // // --- Builder methods ---
    // HttpRequest &&with_method(shared::http::HttpMethod method) && {
    //     add_header(interfaces::io::types::Token::METHOD, method_str(method));
    //     return std::move(*this);
    // }
    // HttpRequest &&with_method(std::string_view method) && {
    //     add_header(interfaces::io::types::Token::METHOD, method);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_path(std::string_view path) && {
    //     add_header(interfaces::io::types::Token::PATH, path);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_scheme(std::string_view schema) && {
    //     add_header(interfaces::io::types::Token::SCHEME, schema);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_authority(std::string_view authority) && {
    //     add_header(interfaces::io::types::Token::AUTHORITY, authority);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_header(std::string_view name, std::string_view value) && {
    //     add_header(name, value);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_header(interfaces::io::types::Token token, std::string_view value) && {
    //     add_header(token, value);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_query(std::string_view key, std::string_view value) && {
    //     auto &path_field =
    //     m_static_headers[std::to_underlying(interfaces::io::types::Token::PATH)]; std::string
    //     new_path; if (path_field) {
    //         new_path = path_field->get_value();
    //     }
    //     new_path += new_path.contains('?') ? '&' : '?';
    //     new_path += utils::encode::url_encode(key);
    //     new_path += '=';
    //     new_path += utils::encode::url_encode(value);
    //     add_header(interfaces::io::types::Token::PATH, new_path);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_bearer_auth(std::string_view token) && {
    //     add_header(interfaces::io::types::Token::AUTHORIZATION, "Bearer " + std::string(token));
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_basic_auth(std::string_view user, std::string_view password) && {
    //     add_header(interfaces::io::types::Token::AUTHORIZATION,
    //                "Basic " + utils::encode::base64_encode(std::string(user) + ":" +
    //                                                        std::string(password)));
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_content_type(std::string_view mime) && {
    //     add_header(interfaces::io::types::Token::CONTENT_TYPE, mime);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_accept(std::string_view mime) && {
    //     add_header(interfaces::io::types::Token::ACCEPT, mime);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_user_agent(std::string_view user) && {
    //     add_header(interfaces::io::types::Token::USER_AGENT, user);
    //     return std::move(*this);
    // }
    //
    // [[nodiscard]] HttpRequest build() && { return std::move(*this); }

    /**
     * @brief Deleted — this holds a swiss hashmap and a fixed-size array of shared header
     * pointers, copying gets messy fast, moving's the only supported way to relocate one of
     * these.
     */
    HttpRequest(const HttpRequest &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor right above.
     */
    HttpRequest &operator=(const HttpRequest &) = delete;
    /**
     * @brief Defaulted move ctor — cheap relocation, no deep header copies involved.
     */
    constexpr HttpRequest(HttpRequest &&) noexcept = default;
    /**
     * @brief Defaulted move assign, matches the move ctor right above.
     */
    constexpr HttpRequest &operator=(HttpRequest &&) noexcept = default;
    /**
     * @brief Defaulted destructor — overrides `IRequest`'s virtual destructor, no extra
     * teardown logic needed beyond the members' own destructors.
     */
    ~HttpRequest() override = default;

    /**
     * @brief `IRequest::set_header()` override — the real header-setting engine for HTTP/2
     * requests. String names get tokenized first; known tokens land in the fixed
     * `m_static_headers` array (COOKIE gets special-cased to concatenate with the RFC-mandated
     * `; ` separator instead of overwriting), unrecognized names fall through to the swiss
     * hashmap `m_headers`.
     * @warning Straight up cooked edge case, not vibes: call this with an unrecognized string
     * name AND an empty `value`. `tokenize()` fails, `!value.empty()` is false so the
     * merge-or-insert-then-`return` branch is skipped, so execution falls through to
     * `m_headers.insert(...)` and then straight into `auto token = token_opt.value();` with
     * `token_opt` holding no value at all. That's an unconditional `std::bad_optional_access`
     * thrown, uncaught, for what looks like a totally reasonable call (set a custom header to
     * an empty string). Every other combination of known/unknown name × empty/non-empty value
     * is fine — it's specifically unknown-name-plus-empty-value that's an L waiting to happen.
     * @param name_or_token the header name, or its interned token.
     * @param value the header value.
     * @throws std::invalid_argument if given an empty string name, or if given `Token::NONE`.
     * @throws std::bad_optional_access if given an unrecognized string name with an empty
     * value — see warning above.
     */
    void set_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token,
                    std::string_view value) &
        override {
        std::visit(
            [this, value](auto &&name) {
                using T = std::decay_t<decltype(name)>;

                // String-name path — needs to tokenize first to figure out where it lands.
                if constexpr (std::is_same_v<T, std::string_view>) {
                    if (name.empty()) {
                        throw std::invalid_argument("Header name cannot be empty");
                    }

                    // Unknown name — goes straight into the dynamic hashmap instead of the
                    // fixed static-header array (see the class-level warning re: empty value here).
                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (!token_opt.has_value()) {
                        if (!value.empty()) {
                            if (auto existing_opt = m_headers.find(name);
                                existing_opt.has_value()) {
                                const auto &existing = *existing_opt;
                                existing->set_value(existing->get_value() +
                                                    interfaces::consts::VALUE_SEPARATOR +
                                                    std::string(value));
                            }
                            return;
                        }
                        m_headers.insert(name, std::make_shared<interfaces::io::HeaderField<false>>(
                                                   name, value));
                    }

                    auto token = token_opt.value();

                    // Known token — COOKIE gets special-cased to concatenate onto whatever's
                    // already there (RFC-mandated "; " separator) instead of overwriting;
                    // everything else just recurses into the token overload below.
                    if (token == interfaces::io::types::Token::COOKIE) {
                        const auto IDX = std::to_underlying(interfaces::io::types::Token::COOKIE);
                        if (m_static_headers[IDX] == nullptr) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                            m_static_headers[IDX] =  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                std::make_shared<interfaces::io::HeaderField<true>>(
                                    interfaces::io::types::Token::COOKIE, std::string(value));
                        } else if (!value.empty()) {
                            m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() +  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                                             interfaces::consts::COOKIE_SEPARATOR +
                                                             std::string(value));
                        }
                    } else {
                        set_header(token, value);
                    }

                } else if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    // Token path — straight write into the matching static-header slot.
                    if (name == interfaces::io::types::Token::NONE) {
                        throw std::invalid_argument("interfaces::io::types::Token cannot be None");
                    }

                    m_static_headers[std::to_underlying(name)] =  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                        std::make_shared<interfaces::io::HeaderField<true>>(name,
                                                                            std::string(value));
                }
            },
            name_or_token);
    }

    /**
     * @brief `IRequest::remove_header()` override — nulls out the slot in `m_static_headers`
     * for a known token (or a string name that tokenizes to one), otherwise erases it from the
     * `m_headers` hashmap.
     * @param name_or_token the header name (or token) to remove.
     */
    void remove_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token) &
        override {
        std::visit(
            [&](const auto &name) {
                using T = std::decay_t<decltype(name)>;
                // Direct token — null out its static-header slot.
                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    m_static_headers[std::to_underlying(name)] = nullptr;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                } else {
                    // String name — tokenizes to a known slot, or falls back to erasing from
                    // the dynamic hashmap.
                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (token_opt.has_value()) {
                        m_static_headers[std::to_underlying(token_opt.value())] = nullptr;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                    } else {
                        m_headers.erase(name);
                    }
                }
            },
            name_or_token);
    }

    /**
     * @brief Estimates the total on-wire size this request will take up once framed — header
     * block (across however many HEADERS+CONTINUATION frames it needs to fit in
     * `max_frame_payload`-sized chunks) plus the DATA frame(s) carrying the body.
     * @note Not part of the `IRequest` contract, this is HTTP/2-specific — used upstream to
     * pre-size the `BufferNode` before actually encoding the request (see `Session::send()`).
     * @note An empty body still counts 9 bytes for a mandatory empty DATA frame carrying
     * END_STREAM — matches the same "always at least one frame" logic
     * `WriteFrameClosureAdapter` enforces when it actually writes the bytes.
     * @param max_frame_payload the local SETTINGS_MAX_FRAME_SIZE, used to figure out how many
     * frames the header block and body each need to split across.
     * @return the estimated total wire size in bytes.
     */
    [[nodiscard]] std::size_t get_size(std::size_t max_frame_payload) const noexcept {
        std::size_t total = 0;

        // Sum up every set static header field's size first.
        std::size_t header_block = std::ranges::fold_left(
            m_static_headers |
                std::views::filter([](const auto &field) noexcept { return field != nullptr; }),
            std::size_t{0},
            [](std::size_t acc, const auto &field) noexcept { return acc + field->size(); });

        // TODO: add ranges support to my swiss hashmap
        //  header_block = std::ranges::fold_left(m_headers, header_block, [](std::size_t acc,
        //  const auto &entry) noexcept {
        //      return acc + entry.value()->size();
        //  });
        // Then fold in every dynamic header too, same running total.
        for (const auto &entry : m_headers) {
            header_block += entry.value()->size();
        }

        // Figure out how many HEADERS+CONTINUATION frames the header block needs (at least 1,
        // even for an empty block) and add each frame's 9-byte header overhead.
        std::size_t num_header_frames =
            (header_block == 0) ? 1 : (header_block + max_frame_payload - 1) / max_frame_payload;

        total += (num_header_frames * 9) + header_block;

        // --- DATA frames ---
        const std::size_t BODY_SIZE = m_body.size();

        // Same chunking math for the body, except an empty body still costs exactly one
        // 9-byte empty DATA frame carrying END_STREAM.
        if (BODY_SIZE > 0) {
            std::size_t num_data_frames = (BODY_SIZE + max_frame_payload - 1) / max_frame_payload;
            total += (num_data_frames * 9) + BODY_SIZE;
        } else {
            total += 9; // empty DATA frame with END_STREAM
        }

        return total;
    }

    /**
     * @brief `IRequest::find_header()` override — checks `m_static_headers` for a known token
     * (or a tokenizable string name) first, falls back to the `m_headers` hashmap for anything
     * that doesn't tokenize.
     * @param name_or_token the header name, or its interned token.
     * @return the header's value, or an empty `string_view` if it isn't set.
     */
    // FIXME(clang-tidy): bugprone-exception-escape — the std::visit-internal throw path is gone
    // (dispatch below is manual get_if, not std::visit), but m_headers.find() (SwissHashMap) is
    // not itself noexcept — its hash/equality functor calls can in theory throw — and `noexcept`
    // here is required to match IRequest::find_header()'s noexcept virtual signature; can't drop
    // it without breaking the override.
    [[nodiscard]] std::string_view
    find_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token)
        const noexcept override {
        // Manual index()/get_if dispatch instead of std::visit — libstdc++'s std::get<>() (which
        // std::visit uses internally) has an unconditional valueless_by_exception() throw check
        // that trips bugprone-exception-escape even though this variant is never valueless;
        // get_if() has no such check.
        if (const auto *token = std::get_if<interfaces::io::types::Token>(&name_or_token)) {
            const auto &field = m_static_headers[std::to_underlying(*token)];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
            return field ? std::string_view{field->get_value()} : std::string_view{};
        }

        const auto &name = *std::get_if<std::string_view>(&name_or_token);

        // String name — check if it tokenizes to a known static slot first...
        auto token_opt = interfaces::io::types::tokenize(name);
        if (token_opt.has_value()) {
            const auto &field = m_static_headers[std::to_underlying(token_opt.value())];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
            return field ? std::string_view{field->get_value()} : std::string_view{};
        }

        // ...otherwise fall back to the dynamic hashmap.
        auto result = m_headers.find(name);
        return result.has_value() ? std::string_view{(*result)->get_value()} : std::string_view{};
    }

    /**
     * @brief `IRequest::clear_headers()` override.
     * @warning This is a straight-up no-op right now, body's empty — the `TODO` says it plain:
     * "add clear functions in qpack else this gets bad." Calling this does NOT actually clear
     * `m_static_headers` or `m_headers`, and it definitely doesn't touch whatever's already
     * sitting in the HPACK dynamic table for this stream. Anyone calling this expecting a clean
     * slate is getting cooked — the headers are all still there afterward, silently.
     */
    void clear_headers() & noexcept override {
        // TODO: add clear functions in qpack else this gets bad fix after compiles
    }

    /**
     * @brief `IRequest::get_headers()` override — collects every non-null static header plus
     * everything in the dynamic `m_headers` hashmap into one flat vector.
     * @return every header currently set on this request, static ones first then dynamic.
     */
    [[nodiscard]] std::vector<interfaces::io::HeaderEntry> get_headers() const noexcept override {
        std::vector<interfaces::io::HeaderEntry> result;
        // Static headers first — skip the empty (unset) slots.
        for (const auto &field : m_static_headers) {
            if (field != nullptr) {
                result.emplace_back(field);
            }
        }
        // Then every dynamic header on top.
        for (const auto &entry : m_headers) {
            result.emplace_back(entry.value());
        }
        return result;
    }

    /**
     * @brief `IRequest::get_body()` override — mutable access.
     * @return a mutable `BufferView` over the body bytes.
     */
    [[nodiscard]] utils::buffering::BufferView &get_body() noexcept override { return m_body; }

    /**
     * @brief `IRequest::get_body()` const override — read-only access.
     * @return a read-only `BufferView` over the body bytes.
     */
    [[nodiscard]] const utils::buffering::BufferView &get_body() const noexcept override {
        return m_body;
    }


    //  TODO: set verion automaticly
    //  virtual void set_version(std::string_view version) & noexcept { std::abort(); }
    // TODO: think of a solution for this!!!
    // virtual void set_no_decompress(bool disable) & noexcept { std::abort(); }
    /**
     * @brief `IRequest::set_addr()` override — sets the AUTHORITY header. Correctly wired to
     * `Token::AUTHORITY`, worth noting only because the connection-level `IRequest::add_addr()`
     * base-class helper has a genuine copy-paste bug where it stomps AUTHORIZATION instead —
     * this override does NOT share that bug, it's right.
     * @param addr the address to set as the AUTHORITY header.
     */
    // FIXME(clang-tidy): bugprone-exception-escape — set_header() is documented to throw
    // (std::invalid_argument, std::bad_optional_access) but `noexcept` here is required to match
    // `IRequest::set_addr()`'s noexcept virtual signature; can't drop it without breaking the
    // override.
    void set_addr(std::string_view addr) & noexcept override {
        set_header(interfaces::io::types::Token::AUTHORITY, addr);
    }

    /**
     * @brief `IRequest::get_method()` override.
     * @return the method string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_method() const noexcept override {
        return find_header(interfaces::io::types::Token::METHOD);
    }
    /**
     * @brief `IRequest::get_path()` override.
     * @return the path string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_path() const noexcept override {
        return find_header(interfaces::io::types::Token::PATH);
    }
    /**
     * @brief `IRequest::get_host()` override.
     * @return the host string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_host() const noexcept override {
        return find_header(interfaces::io::types::Token::HOST);
    }
    /**
     * @brief `IRequest::get_scheme()` override.
     * @return the scheme string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_scheme() const noexcept override {
        return find_header(interfaces::io::types::Token::SCHEME);
    }
    /**
     * @brief `IRequest::get_authority()` override.
     * @return the authority string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_authority() const noexcept override {
        return find_header(interfaces::io::types::Token::AUTHORITY);
    }
    /**
     * @brief `IRequest::get_content_type()` override.
     * @return the content-type string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_content_type() const noexcept override {
        return find_header(interfaces::io::types::Token::CONTENT_TYPE);
    }
    /**
     * @brief `IRequest::get_accept()` override.
     * @return the accept string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_accept() const noexcept override {
        return find_header(interfaces::io::types::Token::ACCEPT);
    }
    /**
     * @brief `IRequest::get_user_agent()` override.
     * @return the user-agent string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_user_agent() const noexcept override {
        return find_header(interfaces::io::types::Token::USER_AGENT);
    }
    /**
     * @brief `IRequest::get_authorization()` override.
     * @return the authorization string, or empty if unset.
     */
    [[nodiscard]] std::string_view get_authorization() const noexcept override {
        return find_header(interfaces::io::types::Token::AUTHORIZATION);
    }


  private:
    // void add_header(std::string_view name, std::string_view value) {
    //     if (name.empty()) {
    //         throw std::invalid_argument("Header name cannot be empty");
    //     }
    //
    //     auto token = interfaces::io::types::tokenize(name);
    //     if (token == interfaces::io::types::Token::COOKIE) {
    //         const auto IDX = std::to_underlying(interfaces::io::types::Token::COOKIE);
    //         if (m_static_headers[IDX] == nullptr) {
    //             m_static_headers[IDX] = std::make_shared<interfaces::io::HeaderField<true>>(
    //                 interfaces::io::types::Token::COOKIE, std::string(value));
    //         } else if (!value.empty()) {
    //             m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() +
    //                                              interfaces::consts::COOKIE_SEPARATOR +
    //                                              std::string(value));
    //         }
    //     } else if (token == interfaces::io::types::Token::CUSTOM) {
    //         if (!value.empty()) {
    //             if (auto existing_opt = m_headers.find(name); existing_opt.has_value()) {
    //                 const auto &existing = *existing_opt;
    //                 existing->set_value(existing->get_value() +
    //                 interfaces::consts::VALUE_SEPARATOR +
    //                                     std::string(value));
    //             }
    //             return;
    //         }
    //         m_headers.add_header(name, std::make_shared<interfaces::io::HeaderField<false>>(name,
    //         value));
    //     } else {
    //         add_header(token, value);
    //     }
    // }
    //
    // void add_header(interfaces::io::types::Token token, std::string_view value) {
    //     if (token == interfaces::io::types::Token::NONE) {
    //         throw std::invalid_argument("interfaces::io::types::Token cannot be None");
    //     }
    //     if (token == interfaces::io::types::Token::CUSTOM) {
    //         throw std::invalid_argument("interfaces::io::types::Token cannot be Custom");
    //     }
    //
    //     m_static_headers[std::to_underlying(token)] =
    //         std::make_shared<interfaces::io::HeaderField<true>>(token, std::string(value));
    // }


    struct FactoryTag {};

    /**
     * @brief Tag-dispatched factory ctor — builds a request pre-loaded with METHOD and PATH in
     * one shot. `FactoryTag` exists purely to disambiguate this overload from the public
     * `HttpRequest(stream_id)` ctor, it carries no data of its own.
     * @param stream_id the stream this request rides on.
     * @param method the HTTP method to set.
     * @param path the request path to set.
     */
    HttpRequest(FactoryTag tag, std::uint32_t stream_id, interfaces::io::types::Method method,
                std::string_view path)
        : HttpRequest{stream_id} {
        // Pre-load the two headers every request needs — method first, then path.
        set_header(interfaces::io::types::Token::METHOD, method_str(method));
        set_header(interfaces::io::types::Token::PATH, path);
    }

    /**
     * @brief Looks up a static header field by its token slot directly, bypassing
     * `find_header()`'s string-view-only return.
     * @param token the token to look up.
     * @return the header field shared_ptr for that slot, or `nullptr` if unset.
     */
    std::shared_ptr<interfaces::io::HeaderField<true>>
    get_static(const interfaces::io::types::Token &token) {
        return m_static_headers[std::to_underlying(token)];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    std::uint32_t m_stream_id{0};
    // TODO: make length a constant
    std::array<std::shared_ptr<interfaces::io::HeaderField<true>>,
               std::to_underlying(interfaces::io::types::Token::WWW_AUTHENTICATE) + 1>
        m_static_headers{};

    hashmap::swiss::SwissHashMap<std::string_view,
                                 std::shared_ptr<interfaces::io::HeaderField<false>>>
        m_headers;

    utils::buffering::BufferView m_body;
};

struct WriteHttpRequestAdaptor : std::ranges::range_adaptor_closure<WriteHttpRequestAdaptor> {
    /**
     * @brief Range adaptor closure ctor — stashes the request to encode, the HPACK table to
     * encode headers against, and the frame-chunking config.
     * @param req the request to encode. Stored by reference, must outlive this adaptor.
     * @param table the HPACK dynamic table to encode header field references against.
     * @param max_frame_size the slice size the header block and body payload each get chunked
     * to.
     * @param flags base flags applied to the DATA frame(s) when the body's non-empty.
     */
    explicit constexpr WriteHttpRequestAdaptor(HttpRequest &req, codec::hpack::HPackTable &table,
                                               std::size_t max_frame_size, std::uint8_t flags = 0)
        : m_req{req}, m_table{table}, m_max_frame_size{max_frame_size}, m_flags{flags} {}

    /**
     * @brief Encodes the full request onto `output` — HPACK-encodes the headers into one or
     * more HEADERS/CONTINUATION frames (END_HEADERS set on the last chunk), then appends either
     * the body as DATA frame(s) or a single empty END_STREAM DATA frame if there's no body.
     * @warning The HPACK encoder's flush callback runs synchronously, mutating `first_frame`
     * across calls — first flush emits HEADERS, every flush after that emits CONTINUATION. Get
     * the encoder's flush ordering wrong somehow and frame types desync from what the peer
     * expects, that's a real footgun in header-block continuation logic generally, not
     * something unique to this call site but worth knowing going in.
     * @note Deduced `auto` return, but there's no `return` statement in the body — this
     * actually resolves to `void`. All the real work happens through `append_range()` calls
     * mutating `output` in place, not through a return value at all.
     * @tparam R a viewable range this appends the encoded request bytes onto.
     * @param output the range to append the encoded request onto — mutated in place.
     */
    template <std::ranges::viewable_range R>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) — output is mutated via append_range() across multiple flush-callback invocations; forwarding it would use-after-move after the first call
    auto operator()(R &&output) const {
        const auto STREAM_ID = m_req.get().get_stream_id();
        auto header_entries = m_req.get().get_headers();

        // Tracks whether the encoder's flush callback has fired yet — first flush emits
        // HEADERS, every one after that emits CONTINUATION.
        bool first_frame = true;

        // HPACK-encode every header entry, letting the encoder decide where the chunk
        // boundaries fall — each chunk gets its own frame header written on flush.
        codec::hpack::HpackEncoder<std::uint32_t>{
            m_table.get(), std::span<const interfaces::io::HeaderEntry>(header_entries),
            m_max_frame_size,
            [&](std::span<const std::byte> data, codec::hpack::HpackFlushReason reason) {
                const auto TYPE = first_frame ? shared_layer::FrameType::HEADERS
                                              : shared_layer::FrameType::CONTINUATION;
                // END_HEADERS only lands on the final chunk — the encoder tells us via `reason`.
                const std::uint8_t FLAGS =
                    (reason == codec::hpack::HpackFlushReason::END)
                        ? static_cast<std::uint8_t>(shared_layer::Flags::END_HEADERS)
                        : std::uint8_t{0};
                // NOTE: this flush callback can fire multiple times (once per HPACK chunk), so
                // `output` — a forwarding reference captured above — must be used as an lvalue
                // here rather than forwarded; forwarding it on every call would make each call
                // after the first a use-after-move.
                output.append_range(
                    std::views::empty<std::byte> |
                    FrameHeaderClosureAdaptor{static_cast<std::uint32_t>(data.size()), TYPE, FLAGS,
                                              STREAM_ID});
                output.append_range(data);
                first_frame = false;
            }}();

        // No body — still gotta close the stream, so emit a single empty DATA frame carrying
        // END_STREAM rather than skipping DATA entirely.
        if (m_req.get().get_body().empty()) {
            std::uint8_t data_flags = m_flags | shared_layer::Flags::END_STREAM;

            auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                             .add_type(shared_layer::FrameType::DATA)
                             .add_flags(data_flags)
                             .add_stream_id(STREAM_ID)
                             .build();

            output.append_range(std::views::empty<std::byte> |
                                WriteFrameBuilderAdaptor{std::move(frame), m_max_frame_size});
        } else {
            // Real body — let WriteFrameClosureAdapter handle the chunking and END_STREAM
            // placement on the last DATA frame.
            output.append_range(m_req.get().get_body() |
                                WriteFrameClosureAdapter{STREAM_ID, shared_layer::FrameType::DATA,
                                                         m_flags, m_max_frame_size});
        }
    }

    std::reference_wrapper<HttpRequest> m_req;
    std::reference_wrapper<codec::hpack::HPackTable> m_table;
    std::size_t m_max_frame_size;
    std::uint8_t m_flags;
};

} // namespace io::layer::http2
