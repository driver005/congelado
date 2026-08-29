export module interfaces:io_response;

import std;
import utils_buffering;
import :io_header;
import :io_types;

export namespace interfaces::io {

// CRTP base for protocol-agnostic responses (HTTP/1–3, gRPC, WebSocket, …).
// All mutators return Derived& for builder chaining.
class IResponse {
  public:
    /**
     * @brief Builds a response tied to a stream id. Nothing else set yet, just the id.
     * @param stream_id the stream this response rides on.
     */
    IResponse(std::uint32_t stream_id) : m_stream_id{stream_id} {}
    /**
     * @brief Default ctor, just delegates to stream id 0.
     */
    IResponse() : IResponse{0} {};

    /**
     * @brief Copy ctor — plain value copy, no resources here that'd need special handling.
     */
    IResponse(const IResponse &) = default;
    /**
     * @brief Copy assignment — plain value copy, no resources here that'd need special handling.
     */
    IResponse &operator=(const IResponse &) = default;
    /**
     * @brief Move ctor — plain value move, no resources here that'd need special handling.
     */
    IResponse(IResponse &&) = default;
    /**
     * @brief Move assignment — plain value move, no resources here that'd need special handling.
     */
    IResponse &operator=(IResponse &&) = default;

    /**
     * @brief Virtual dtor so derived response types clean up right through the base pointer,
     * no leaks left behind.
     */
    virtual ~IResponse() = default;

    /**
     * @brief Spins up a fresh 200 OK response. The W status, straight vibes.
     * @param stream_id the stream id to tag the new response with.
     * @return a heap-allocated response with status OK already set.
     */
    [[nodiscard]] static std::unique_ptr<IResponse> ok(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::OK);
        return res;
    }

    /**
     * @brief Spins up a fresh 201 Created response.
     * @param stream_id the stream id to tag the new response with.
     * @return a heap-allocated response with status CREATED already set.
     */
    [[nodiscard]] static std::unique_ptr<IResponse> created(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::CREATED);
        return res;
    }

    /**
     * @brief Spins up a fresh 204 No Content response. Nothing to see here, and that's the point.
     * @param stream_id the stream id to tag the new response with.
     * @return a heap-allocated response with status NO_CONTENT already set.
     */
    [[nodiscard]] static std::unique_ptr<IResponse> no_content(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::NO_CONTENT);
        return res;
    }

    /**
     * @brief Spins up a fresh 400 Bad Request response. Somebody sent something cooked.
     * @param stream_id the stream id to tag the new response with.
     * @return a heap-allocated response with status BAD_REQUEST already set.
     */
    [[nodiscard]] static std::unique_ptr<IResponse> bad_request(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::BAD_REQUEST);
        return res;
    }

    /**
     * @brief Spins up a fresh 404 Not Found response. It's just not there, no cap.
     * @param stream_id the stream id to tag the new response with.
     * @return a heap-allocated response with status NOT_FOUND already set.
     */
    [[nodiscard]] static std::unique_ptr<IResponse> not_found(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::NOT_FOUND);
        return res;
    }

    /**
     * @brief Spins up a fresh 500 Internal Server Error response. Somewhere, something's on
     * fire and it's not the caller's fault.
     * @param stream_id the stream id to tag the new response with.
     * @return a heap-allocated response with status INTERNAL_SERVER_ERROR already set.
     */
    [[nodiscard]] static std::unique_ptr<IResponse> internal_error(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::INTERNAL_SERVER_ERROR);
        return res;
    }

    /**
     * @brief Builder chain — sets a header by string name. Works for whatever header you throw
     * at it.
     * @param name the header name.
     * @param value the header value.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_header(std::string_view name, std::string_view value) && noexcept {
        set_header(name, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets a header by interned `Token`, quicker path when you've got
     * one on hand.
     * @param token the header's token.
     * @param value the header value.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_header(types::Token token, std::string_view value) && noexcept {
        set_header(token, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — drops a header by name if it's set, no-op otherwise.
     * @param name the header name to remove.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&without_header(std::string_view name) && noexcept {
        remove_header(name);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the response status. The whole W-or-L of the response,
     * basically.
     * @param status the status to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_status(types::Status status) && noexcept {
        set_status(status);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the response body from any input range of `std::byte`.
     * @tparam R an input range whose value type is `std::byte`.
     * @param body_range the byte range to install as the body.
     * @return `*this`, moved, so the chain keeps going.
     */
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    IResponse &&with_body(R &&body_range) && noexcept {
        set_body(std::forward<R>(body_range));
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the stream id.
     * @param stream_id the stream id to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_stream_id(std::uint32_t stream_id) && noexcept {
        set_stream_id(stream_id);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — stashes the method on this response, mostly just for reflecting
     * request context back at the caller, since a response doesn't really have a "method" of
     * its own semantically — kinda borrowed vibes from the request side.
     * @param method the method to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_method(types::Method method) && noexcept {
        set_header(types::Token::METHOD, method_str(method));
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Content-Type header.
     * @param content_type the mime type to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_content_type(std::string_view content_type) && noexcept {
        set_header(types::Token::CONTENT_TYPE, content_type);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Content-Length header from a byte count.
     * @param length the content length to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_content_length(std::size_t length) && noexcept {
        set_header(types::Token::CONTENT_LENGTH, std::to_string(length));
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Location header (redirects, resource pointers, etc), the
     * "go look over here instead" header.
     * @param location the location to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_location(std::string_view location) && noexcept {
        set_header(types::Token::LOCATION, location);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the ETag header.
     * @param etag the etag value to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_e_tag(std::string_view etag) && noexcept {
        set_header(types::Token::E_TAG, etag);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Date header.
     * @param date the date value to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_date(std::string_view date) && noexcept {
        set_header(types::Token::DATE, date);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Server header.
     * @param server the server value to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_server(std::string_view server) && noexcept {
        set_header(types::Token::SERVER, server);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Cache-Control header.
     * @param cache_control the cache-control directive(s) to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_cache_control(std::string_view cache_control) && noexcept {
        set_header(types::Token::CACHE_CONTROL, cache_control);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Last-Modified header.
     * @param last_modified the last-modified value to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_last_modified(std::string_view last_modified) && noexcept {
        set_header(types::Token::LAST_MODIFIED, last_modified);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Set-Cookie header. Client's about to be carrying that
     * around for a while.
     * @param cookie the cookie value to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_set_cookie(std::string_view cookie) && noexcept {
        set_header(types::Token::SET_COOKIE, cookie);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — toggles keep-alive for this response's connection.
     * @param keep_alive true to keep the connection alive, false to close it after.
     * @return `*this`, moved, so the chain keeps going.
     */
    IResponse &&with_keep_alive(bool keep_alive) && noexcept {
        set_keep_alive(keep_alive);
        return std::move(*this);
    }

    /**
     * @brief Terminal builder call — closes out the chain, nothing else happens here, just
     * hands back what you already built.
     * @return `*this`, moved out to the caller as the finished response.
     */
    [[nodiscard]] IResponse &&build() && noexcept { return std::move(*this); }

    /**
     * @brief In-place version of with_method().
     * @param method the method to set.
     */
    void add_method(types::Method method) & noexcept {
        set_header(types::Token::METHOD, method_str(method));
    }

    /**
     * @brief In-place version of with_content_type().
     * @param type the mime type to set.
     */
    void add_content_type(std::string_view type) & noexcept {
        set_header(types::Token::CONTENT_TYPE, type);
    }

    /**
     * @brief In-place version of with_content_length().
     * @param length the content length to set.
     */
    void add_content_length(std::size_t length) & noexcept {
        set_header(types::Token::CONTENT_LENGTH, std::to_string(length));
    }

    /**
     * @brief In-place version of with_location().
     * @param location the location to set.
     */
    void add_location(std::string_view location) & noexcept {
        set_header(types::Token::LOCATION, location);
    }

    /**
     * @brief In-place version of with_e_tag().
     * @param etag the etag value to set.
     */
    void add_e_tag(std::string_view etag) & noexcept { set_header(types::Token::E_TAG, etag); }

    /**
     * @brief In-place version of with_date().
     * @param date the date value to set.
     */
    void add_date(std::string_view date) & noexcept { set_header(types::Token::DATE, date); }

    /**
     * @brief In-place version of with_server().
     * @param server the server value to set.
     */
    void add_server(std::string_view server) & noexcept {
        set_header(types::Token::SERVER, server);
    }

    /**
     * @brief In-place version of with_cache_control().
     * @param cache_control the cache-control directive(s) to set.
     */
    void add_cache_control(std::string_view cache_control) & noexcept {
        set_header(types::Token::CACHE_CONTROL, cache_control);
    }

    /**
     * @brief In-place version of with_last_modified().
     * @param last_modified the last-modified value to set.
     */
    void add_last_modified(std::string_view last_modified) & noexcept {
        set_header(types::Token::LAST_MODIFIED, last_modified);
    }

    /**
     * @brief In-place version of with_set_cookie() — but peep the body, it routes through
     * add_header() instead of calling set_header() directly like literally every other add_*
     * method in this class does. Different path, same end result, just inconsistent style —
     * worth knowing about if you're ever chasing down where a header actually gets set.
     * @param cookie the cookie value to set.
     */
    void add_set_cookie(std::string_view cookie) & noexcept {
        add_header(types::Token::SET_COOKIE, cookie);
    }

    /**
     * @brief In-place version of with_status().
     * @param status the status to set.
     */
    void add_status(types::Status status) & noexcept { set_status(status); }

    /**
     * @brief In-place version of with_keep_alive().
     * @param keep_alive true to keep the connection alive, false to close it after.
     */
    void add_keep_alive(bool keep_alive) & noexcept { set_keep_alive(keep_alive); }

    /**
     * @brief In-place version of with_body() — sets the response body from any input range of
     * `std::byte`.
     * @tparam R an input range whose value type is `std::byte`.
     * @param body_range the byte range to install as the body.
     */
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    void add_body(R &&body_range) & noexcept {
        set_body(std::forward<R>(body_range));
    }

    /**
     * @brief In-place version of with_stream_id().
     * @param stream_id the stream id to set.
     */
    void add_stream_id(std::uint32_t stream_id) & noexcept { set_stream_id(stream_id); }

    /**
     * @brief In-place version of with_header() — takes either a name or a `Token`, whichever's
     * around.
     * @param name_or_token the header name, or its interned token.
     * @param value the header value.
     */
    void add_header(std::variant<std::string_view, types::Token> name_or_token,
                    std::string_view value) & noexcept {
        set_header(name_or_token, value);
    }

    /**
     * @brief In-place version of without_header() — drops a header by name or `Token`.
     * @param name the header name (or token) to remove.
     */
    void pop_header(std::variant<std::string_view, types::Token> name) & noexcept {
        remove_header(name);
    }

    /**
     * @brief In-place version of replace_header() — removes then re-sets, by name or `Token`.
     * @param name the header name (or token) to replace.
     * @param value the new value.
     */
    void replace_header(std::variant<std::string_view, types::Token> name,
                        std::string_view value) & noexcept {
        remove_header(name);
        set_header(name, value);
    }


    /**
     * @brief Sets the stream id directly on the member, no header round-trip involved.
     * @param stream_id the stream id to store.
     */
    void set_stream_id(std::uint32_t stream_id) & noexcept { m_stream_id = stream_id; }
    /**
     * @brief Grabs the stream id.
     * @return the stream id this response is tagged with.
     */
    [[nodiscard]] std::uint32_t get_stream_id() const noexcept { return m_stream_id; }

    /**
     * @brief Sets the response body, taking full ownership of `body`.
     * @warning Base impl is just `std::abort()`, straight crash, zero fallback. This MUST be
     * overridden by every concrete response type, and heads up — every other set_body overload
     * floating around this class (the range-based template right below included) all funnel
     * down into this exact one eventually. Skip overriding this and it's cooked the moment
     * anything tries to set a body.
     * @param body the bytes to install as the response body.
     */
    // FIXME(clang-tidy): performance-unnecessary-value-param — by-value is the genuine
    // sink/move-from signature every real override (e.g. io::http2::Response::set_body() in
    // res.cppm) relies on; changing it here without updating every `override` elsewhere would
    // break virtual dispatch, so it's left as-is.
    virtual void set_body(std::vector<std::byte> body) & { std::abort(); }  // NOLINT(performance-unnecessary-value-param) — signature must match every override for virtual dispatch

    /**
     * @brief Sets the response body from any input range of `std::byte` — materializes it into
     * a real `std::vector<std::byte>` and forwards straight to the virtual set_body() overload
     * right above. Convenience wrapper, nothing more.
     * @tparam R an input range whose value type is `std::byte` (excludes
     * `std::vector<std::byte>` itself so overload resolution doesn't get ambiguous and start
     * throwing a fit).
     * @param body_range the byte range to copy into the body.
     */
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte> &&
                 (!std::same_as<std::remove_cvref_t<R>,
                                std::vector<std::byte>>) // Prevents ambiguous overload resolution
    void set_body(R &&body_range) & {
        set_body(std::ranges::to<std::vector<std::byte>>(std::forward<R>(body_range)));
    }

    /**
     * @brief Sets the response status code.
     * @warning Base impl aborts — mandatory override, this whole "Virtual Interface" block is
     * abort-by-default so every concrete response type actually earns its keep.
     * @param status the status to set.
     */
    virtual void set_status(types::Status status) & { std::abort(); }
    /**
     * @brief Toggles keep-alive for this response's connection.
     * @warning Base impl aborts — mandatory override.
     * @param keep_alive true to keep the connection alive, false to close it after.
     */
    virtual void set_keep_alive(bool keep_alive) & { std::abort(); }

    /**
     * @brief Sets a header by name or `Token` — the real customization point, the one every
     * `with_header`/`add_header` overload up above ultimately funnels down into.
     * @warning Base impl aborts — mandatory override, no silent no-op fallback to bail you out.
     * @param name_or_token the header name, or its interned token.
     * @param value the header value.
     */
    virtual void set_header(std::variant<std::string_view, types::Token> name_or_token,
                            std::string_view value) & {
        std::abort();
    }

    /**
     * @brief Removes a header by name or `Token`.
     * @warning Base impl aborts — mandatory override, same as set_header() right above.
     * @param name the header name (or token) to remove.
     */
    virtual void remove_header(std::variant<std::string_view, types::Token> name) & {
        std::abort();
    }

    /**
     * @brief Looks up a header's value by name or `Token`.
     * @warning Base impl aborts — mandatory override.
     * @param name_or_token the header name, or its interned token.
     * @return the header's value if it's set.
     */
    [[nodiscard]] virtual std::string_view
    find_header(std::variant<std::string_view, types::Token> name_or_token) const noexcept {
        std::abort();
    }

    /**
     * @brief Grabs the response status. W, L, or somewhere in between, straight from the enum.
     * @warning Base impl aborts — mandatory override.
     * @return the status code set on this response.
     */
    [[nodiscard]] virtual types::Status get_status() const noexcept { std::abort(); }
    /**
     * @brief Grabs a mutable view over the response body, so callers can actually poke at it.
     * @warning Base impl aborts — mandatory override.
     * @return a mutable `BufferView` over the body bytes.
     */
    [[nodiscard]] virtual utils::buffering::BufferView &get_body() noexcept { std::abort(); }
    /**
     * @brief Const overload — grabs a read-only view over the response body, look but don't touch.
     * @warning Base impl aborts — mandatory override.
     * @return a read-only `BufferView` over the body bytes.
     */
    [[nodiscard]] virtual const utils::buffering::BufferView &get_body() const noexcept {
        std::abort();
    }
    /**
     * @brief Grabs every header currently set on this response, the whole collection.
     * @warning Base impl aborts — mandatory override.
     * @return all headers as a vector of `HeaderEntry`.
     */
    [[nodiscard]] virtual std::vector<HeaderEntry> get_headers() const noexcept { std::abort(); }
    /**
     * @brief Grabs just the Set-Cookie headers off this response, filtered out from the rest.
     * @warning Base impl aborts — mandatory override.
     * @return the Set-Cookie entries as a vector of `HeaderEntry`.
     */
    [[nodiscard]] virtual std::vector<HeaderEntry> get_set_cookies() const noexcept {
        std::abort();
    }
    /**
     * @brief Grabs the human-readable status text ("OK", "Not Found", etc) for this response's
     * status code — the friendly version, not the raw number.
     * @warning Base impl aborts — mandatory override.
     * @return the status text.
     */
    [[nodiscard]] virtual std::string_view get_status_text() const noexcept { std::abort(); }
    /**
     * @brief Grabs the content-type header.
     * @warning Base impl aborts — mandatory override.
     * @return the content-type string.
     */
    [[nodiscard]] virtual std::string_view get_content_type() const noexcept { std::abort(); }
    /**
     * @brief Grabs the content-length header, parsed to an actual number instead of a string.
     * @warning Base impl aborts — mandatory override.
     * @return the content length.
     */
    [[nodiscard]] virtual std::size_t get_content_length() const noexcept { std::abort(); }
    /**
     * @brief Grabs the location header.
     * @warning Base impl aborts — mandatory override.
     * @return the location string.
     */
    [[nodiscard]] virtual std::string_view get_location() const noexcept { std::abort(); }
    /**
     * @brief Grabs the etag header.
     * @warning Base impl aborts — mandatory override.
     * @return the etag string.
     */
    [[nodiscard]] virtual std::string_view get_etag() const noexcept { std::abort(); }
    /**
     * @brief Grabs the date header.
     * @warning Base impl aborts — mandatory override.
     * @return the date string.
     */
    [[nodiscard]] virtual std::string_view get_date() const noexcept { std::abort(); }
    /**
     * @brief Grabs the server header.
     * @warning Base impl aborts — mandatory override.
     * @return the server string.
     */
    [[nodiscard]] virtual std::string_view get_server() const noexcept { std::abort(); }
    /**
     * @brief Grabs the cache-control header.
     * @warning Base impl aborts — mandatory override.
     * @return the cache-control string.
     */
    [[nodiscard]] virtual std::string_view get_cache_control() const noexcept { std::abort(); }
    /**
     * @brief Grabs the last-modified header.
     * @warning Base impl aborts — mandatory override, still no exceptions to the rule.
     * @return the last-modified string.
     */
    [[nodiscard]] virtual std::string_view get_last_modified() const noexcept { std::abort(); }
    /**
     * @brief Checks whether this response keeps the connection alive after it's sent, or slams
     * it shut right after.
     * @warning Base impl aborts — mandatory override.
     * @return true if the connection should stay open, false if it closes after.
     */
    [[nodiscard]] virtual bool is_keep_alive() const noexcept { std::abort(); }
    /**
     * @brief Checks whether the status falls in the 1xx informational range.
     * @warning Base impl aborts — mandatory override.
     * @return true if the status is informational (1xx).
     */
    [[nodiscard]] virtual bool is_informational() const noexcept { std::abort(); }
    /**
     * @brief Checks whether the status falls in the 2xx success range — the actual W zone.
     * @warning Base impl aborts — mandatory override.
     * @return true if the status is a success (2xx).
     */
    [[nodiscard]] virtual bool is_success() const noexcept { std::abort(); }
    /**
     * @brief Checks whether the status falls in the 3xx redirection range.
     * @warning Base impl aborts — mandatory override.
     * @return true if the status is a redirection (3xx).
     */
    [[nodiscard]] virtual bool is_redirection() const noexcept { std::abort(); }
    /**
     * @brief Checks whether the status falls in the 4xx client error range — the caller messed
     * up, not the server.
     * @warning Base impl aborts — mandatory override.
     * @return true if the status is a client error (4xx).
     */
    [[nodiscard]] virtual bool is_client_error() const noexcept { std::abort(); }
    /**
     * @brief Checks whether the status falls in the 5xx server error range — the server's the
     * one that's cooked this time, not the caller.
     * @warning Base impl aborts — mandatory override, last one in the class, you made it.
     * @return true if the status is a server error (5xx).
     */
    [[nodiscard]] virtual bool is_server_error() const noexcept { std::abort(); }

  private:
    std::uint32_t m_stream_id;
};

} // namespace interfaces::io
