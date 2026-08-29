module;
#include <cassert>
#include <ranges>
export module io_layer_http2:response;

import std;
import hashmap;
import shared;
import interfaces;
import io_layer_shared;
import io_codec_hpack;
import utils_buffering;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpResponse : public interfaces::io::IResponse {
  public:
    /**
     * @brief Builds an HTTP/2 response for the given stream, static-header array starts fully
     * empty (all null shared_ptrs).
     * @param stream_id the stream this response rides on, forwarded straight to `IResponse`.
     */
    explicit HttpResponse(std::uint32_t stream_id)
        : interfaces::io::IResponse{stream_id}, m_static_headers{} {}

    /**
     * @brief Deleted — this holds a swiss hashmap and a fixed-size array of shared header
     * pointers, copying gets messy fast, moving's the only supported way to relocate one of
     * these.
     */
    HttpResponse(const HttpResponse &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor right above.
     */
    HttpResponse &operator=(const HttpResponse &) = delete;
    /**
     * @brief Defaulted move ctor — cheap relocation, no deep header copies involved.
     */
    constexpr HttpResponse(HttpResponse &&) noexcept = default;
    /**
     * @brief Defaulted move assign, matches the move ctor right above.
     */
    constexpr HttpResponse &operator=(HttpResponse &&) noexcept = default;
    /**
     * @brief Defaulted destructor override — no resources of its own to release beyond what
     * `IResponse`'s virtual destructor already handles.
     */
    ~HttpResponse() override = default;

    /**
     * @brief `IResponse::set_header()` override — the real header-setting engine for HTTP/2
     * responses. String names get tokenized first; known tokens land in the fixed
     * `m_static_headers` array (COOKIE gets special-cased to concatenate with the RFC-mandated
     * `; ` separator instead of overwriting), unrecognized names fall through to the swiss
     * hashmap `m_headers`.
     * @warning Same cooked edge case as `HttpRequest::set_header()` in req.cppm, identical
     * logic copy-pasted here: call this with an unrecognized string name AND an empty `value`
     * and it falls through past the merge-or-insert-then-`return` branch straight into
     * `token_opt.value()` with `token_opt` holding no value. That's an unconditional
     * `std::bad_optional_access`, uncaught, for what looks like a perfectly normal call.
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

                // String-name overload — has to tokenize before it knows where to store this.
                if constexpr (std::is_same_v<T, std::string_view>) {
                    if (name.empty()) {
                        throw std::invalid_argument("Header name cannot be empty");
                    }

                    // No matching token — lands in the dynamic hashmap (see class-level warning
                    // for the empty-value edge case here).
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

                    // COOKIE concatenates onto the existing value instead of overwriting it;
                    // every other known token just recurses into the token overload below.
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
                    // Token overload — direct write into the static-header array slot.
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
     * @brief `IResponse::remove_header()` override — nulls out the slot in `m_static_headers`
     * for a known token (or a string name that tokenizes to one), otherwise erases it from the
     * `m_headers` hashmap.
     * @param name_or_token the header name (or token) to remove.
     */
    void remove_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token) &
        override {
        std::visit(
            [&](const auto &name) {
                using T = std::decay_t<decltype(name)>;
                // Direct token — clear its static slot.
                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    m_static_headers[std::to_underlying(name)] = nullptr;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                } else {
                    // String name — resolve to a static slot if possible, else erase dynamically.
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
     * @brief `IResponse::set_status()` override — sets the STATUS pseudo-header from a status
     * code enum, converting it to the string HTTP/2 wants on the wire.
     * @param status the status to set.
     */
    void set_status(interfaces::io::types::Status status) & override {
        set_header(interfaces::io::types::Token::STATUS,
                   std::to_string(interfaces::io::types::status_code(status)));
    }

    /**
     * @brief `IResponse::get_headers()` override — collects every non-null static header plus
     * everything in the dynamic `m_headers` hashmap into one flat vector.
     * @return every header currently set on this response, static ones first then dynamic.
     */
    [[nodiscard]] std::vector<interfaces::io::HeaderEntry> get_headers() const noexcept override {
        std::vector<interfaces::io::HeaderEntry> result;
        // Static slots first, skipping the ones that were never set.
        for (const auto &field : m_static_headers) {
            if (field != nullptr) {
                result.emplace_back(field);
            }
        }
        // Dynamic headers appended after, bet.
        for (const auto &entry : m_headers) {
            result.emplace_back(entry.value());
        }
        return result;
    }

    /**
     * @brief Returns an approximation of the total on-wire size of the complete HTTP/2 response
     * (header frames + data frames + payload).
     * @note Not part of the `IResponse` contract, this is HTTP/2-specific — same estimation
     * shape as `HttpRequest::get_size()` over in req.cppm, used to pre-size the `BufferNode`
     * before actually encoding the response (see `Session::response()`).
     * @note An empty body still counts 9 bytes for a mandatory empty DATA frame carrying
     * END_STREAM, matching what `WriteFrameClosureAdapter` actually emits on the wire.
     * @param max_frame_payload the local SETTINGS_MAX_FRAME_SIZE, used to figure out how many
     * frames the header block and body each need to split across.
     * @return the estimated total wire size in bytes.
     */
    [[nodiscard]] std::size_t get_size(const std::size_t &max_frame_payload) const noexcept {
        std::size_t total = 0;

        // Total up every set static header field.
        std::size_t header_block = std::ranges::fold_left(
            m_static_headers |
                std::views::filter([](const auto &field) noexcept { return field != nullptr; }),
            std::size_t{0},
            [](std::size_t acc, const auto &field) noexcept { return acc + field->size(); });

        // TODO: add ranges support to my swiss hashmap
        //  header_block = std::ranges::fold_left(m_headers, header_block, [](std::size_t acc, const
        //  auto &entry) noexcept {
        //      return acc + entry.value()->size();
        //  });
        // Fold in the dynamic headers onto the same running total.
        for (const auto &entry : m_headers) {
            header_block += entry.value()->size();
        }

        // At least 1 header frame even for an empty block, plus each frame's 9-byte overhead.
        std::size_t num_header_frames =
            (header_block == 0) ? 1 : (header_block + max_frame_payload - 1) / max_frame_payload;

        total += (num_header_frames * 9) + header_block;

        // --- DATA frames ---
        const std::size_t BODY_SIZE = m_body.size();

        // Non-empty body chunks the normal way; an empty body still costs one 9-byte DATA
        // frame carrying END_STREAM.
        if (BODY_SIZE > 0) {
            std::size_t num_data_frames = (BODY_SIZE + max_frame_payload - 1) / max_frame_payload;
            total += (num_data_frames * 9) + BODY_SIZE;
        } else {
            total += 9; // empty DATA frame with END_STREAM
        }

        return total;
    }

    /**
     * @brief `IResponse::set_body()` override — adopts `body`'s buffer into a `BufferNode` (O(1)
     * move, no byte copy) and installs it as this response's body chain.
     * @note An empty `body` leaves `m_body` untouched (no node pushed), matching the
     * `get_body().empty()` checks in `WriteHttpResponseAdaptor`.
     * @param body the bytes to install as the response body.
     */
    void set_body(std::vector<std::byte> body) & noexcept override {
        if (body.empty()) {
            return;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) — ref-counted acquire()/release() owns it.
        auto *node = new utils::buffering::BufferNode{std::move(body)};
        m_body.push_back(node, 0, node->get_written());
    }

    /**
     * @brief `IResponse::get_body()` override — mutable access.
     * @return a mutable `BufferView` over the body bytes.
     */
    [[nodiscard]] utils::buffering::BufferView &get_body() noexcept override { return m_body; }

    /**
     * @brief `IResponse::get_body()` const override — read-only access.
     * @return a read-only `BufferView` over the body bytes.
     */
    [[nodiscard]] const utils::buffering::BufferView &get_body() const noexcept override {
        return m_body;
    }

  private:
    /**
     * @brief Alternate ctor that seeds the STATUS header straight from a `Status` enum.
     * @warning Doesn't forward to `IResponse{stream_id}` or the public
     * `HttpResponse(stream_id)` ctor at all — falls through to the base's default ctor, which
     * sets `m_stream_id` to 0. Stream 0 is the reserved connection-level stream in HTTP/2, not
     * a valid per-request stream id, so building a response through this ctor and actually
     * sending it on a real stream would tag it wrong. Currently unused anywhere in the
     * codebase (grep confirms zero call sites), so it's dormant rather than actively broken —
     * but wiring a caller up to this one without also setting the stream id after the fact
     * would be a real L.
     * @param status the status to seed the STATUS header with.
     */
    HttpResponse(interfaces::io::types::Status status) {
        set_header(interfaces::io::types::Token::STATUS,
                   std::to_string(interfaces::io::types::status_code(status)));
    }

    /**
     * @brief Looks up a static header field by its token slot directly, bypassing whatever a
     * `find_header()`-style string-view-only accessor would give you.
     * @param token the token to look up.
     * @return the header field shared_ptr for that slot, or `nullptr` if unset.
     */
    std::shared_ptr<interfaces::io::HeaderField<true>>
    get_static(const interfaces::io::types::Token &token) {
        return m_static_headers[std::to_underlying(token)];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    // TODO: make length a constant
    std::array<std::shared_ptr<interfaces::io::HeaderField<true>>,
               std::to_underlying(interfaces::io::types::Token::WWW_AUTHENTICATE) + 1>
        m_static_headers{};
    hashmap::swiss::SwissHashMap<std::string_view,
                                 std::shared_ptr<interfaces::io::HeaderField<false>>>
        m_headers;
    utils::buffering::BufferView m_body;
};

struct WriteHttpResponseAdaptor : std::ranges::range_adaptor_closure<WriteHttpResponseAdaptor> {
    /**
     * @brief Range adaptor closure ctor — stashes the response to encode, the HPACK table to
     * encode headers against, and the frame-chunking config.
     * @param res the response to encode. Stored by reference, must outlive this adaptor.
     * @param table the HPACK dynamic table to encode header field references against.
     * @param max_frame_size the slice size the header block and body payload each get chunked
     * to.
     * @param flags base flags applied to the DATA frame(s) when the body's non-empty.
     */
    explicit constexpr WriteHttpResponseAdaptor(HttpResponse &res, codec::hpack::HPackTable &table,
                                                std::size_t max_frame_size, std::uint8_t flags = 0)
        : m_res{res}, m_table{table}, m_max_frame_size{max_frame_size}, m_flags{flags} {}

    /**
     * @brief Encodes the full response onto `output` — HPACK-encodes the headers into one or
     * more HEADERS/CONTINUATION frames (END_HEADERS set on the last chunk), then appends either
     * the body as DATA frame(s) or a single empty END_STREAM DATA frame if there's no body.
     * @warning Mirrors `WriteHttpRequestAdaptor::operator()` in req.cppm exactly, same
     * synchronous-flush-callback footgun applies — first HPACK flush emits HEADERS, every one
     * after emits CONTINUATION, driven by a mutable `first_frame` captured in the encoder's
     * callback.
     * @note Deduced `auto` return, but there's no `return` statement in the body — this
     * actually resolves to `void`. All the real work happens through `append_range()` calls
     * mutating `output` in place.
     * @tparam R a viewable range this appends the encoded response bytes onto.
     * @param output the range to append the encoded response onto — mutated in place.
     */
    // FIXME(clang-tidy): cppcoreguidelines-missing-std-forward — `output` is mutated in place via
    // append_range() throughout this function, never forwarded on to another function, so
    // std::forward would be a no-op here.
    template <std::ranges::viewable_range R>
    auto operator()(R &&output) const {  // NOLINT(cppcoreguidelines-missing-std-forward) — signature must match every override for virtual dispatch
        const auto STREAM_ID = m_res.get().get_stream_id();
        auto header_entries = m_res.get().get_headers();

        // First encoder flush emits HEADERS, every one after that flips to CONTINUATION.
        bool first_frame = true;

        // Run the headers through HPACK — the encoder decides chunk boundaries and calls back
        // once per chunk, each callback writing that chunk's frame header + bytes straight
        // onto output.
        codec::hpack::HpackEncoder<std::uint32_t>{
            m_table.get(), std::span<const interfaces::io::HeaderEntry>(header_entries),
            m_max_frame_size,
            [&](std::span<const std::byte> data, codec::hpack::HpackFlushReason reason) {
                const auto TYPE = first_frame ? shared_layer::FrameType::HEADERS
                                              : shared_layer::FrameType::CONTINUATION;
                // Only the last chunk gets END_HEADERS — the encoder signals that via `reason`.
                const std::uint8_t FLAGS =
                    (reason == codec::hpack::HpackFlushReason::END)
                        ? static_cast<std::uint8_t>(shared_layer::Flags::END_HEADERS)
                        : std::uint8_t{0};
                output.append_range(
                    std::views::empty<std::byte> |
                    FrameHeaderClosureAdaptor{static_cast<std::uint32_t>(data.size()), TYPE, FLAGS,
                                              STREAM_ID});
                output.append_range(data);
                first_frame = false;
            }}();

        // Empty body still needs to close the stream — one empty DATA frame with END_STREAM
        // instead of skipping DATA altogether.
        if (m_res.get().get_body().empty()) {
            std::uint8_t data_flags = m_flags | shared_layer::Flags::END_STREAM;

            auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                             .add_type(shared_layer::FrameType::DATA)
                             .add_flags(data_flags)
                             .add_stream_id(STREAM_ID)
                             .build();

            output.append_range(std::views::empty<std::byte> |
                                WriteFrameBuilderAdaptor{std::move(frame), m_max_frame_size});
        } else {
            // Real body — hand off to WriteFrameClosureAdapter for chunking + END_STREAM on
            // the last DATA frame.
            output.append_range(m_res.get().get_body() |
                                WriteFrameClosureAdapter{STREAM_ID, shared_layer::FrameType::DATA,
                                                         m_flags, m_max_frame_size});
        }
    }

    std::reference_wrapper<HttpResponse> m_res;
    std::reference_wrapper<codec::hpack::HPackTable> m_table;
    std::size_t m_max_frame_size;
    std::uint8_t m_flags;
};

} // namespace io::layer::http2
