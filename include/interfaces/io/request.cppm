export module interfaces:io_request;

import std;
import utils_buffering;
import utils_encode;
import :io_types;
import :io_header;

export namespace interfaces::io {

// CRTP base for protocol-agnostic requests (HTTP/1–3, gRPC, WebSocket, …).
// Protocols without header/body support inherit no-op defaults.
// All mutators return Derived& for builder chaining.
class IRequest {
  public:
    /**
     * @brief Builds a request tied to a stream id, timeout starts at zero — no cap set yet, that
     * part's on you to configure later.
     * @param stream_id the stream this request rides on.
     */
    IRequest(std::uint32_t stream_id)
        : m_stream_id{stream_id}, m_timeout{std::chrono::milliseconds::zero()} {}
    /**
     * @brief Default ctor, just delegates to stream id 0. Nothing deep here.
     */
    IRequest() : IRequest{0} {};

    /**
     * @brief Copy ctor — plain value copy, no resources here that'd need special handling.
     */
    IRequest(const IRequest &) = default;
    /**
     * @brief Copy assignment — plain value copy, no resources here that'd need special handling.
     */
    IRequest &operator=(const IRequest &) = default;
    /**
     * @brief Move ctor — plain value move, no resources here that'd need special handling.
     */
    IRequest(IRequest &&) = default;
    /**
     * @brief Move assignment — plain value move, no resources here that'd need special handling.
     */
    IRequest &operator=(IRequest &&) = default;

    /**
     * @brief Virtual dtor so derived request types clean up right through the base pointer, no
     * leaks left behind.
     */
    virtual ~IRequest() = default;

    /**
     * @brief Spins up a fresh GET request pointed at `path`. Straightforward, bet.
     * @warning Not static despite never touching `this` — calling it on any instance gives the
     * exact same result, which is kinda sus ngl, but that's how it's written. Don't go reading
     * meaning into which instance you happen to call it on, there isn't any.
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with GET + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> get(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::GET).with_path(path);
        return req;
    }
    /**
     * @brief Spins up a fresh HEAD request pointed at `path`.
     * @warning Same non-static quirk as get() right above — doesn't touch `this` at all, works
     * the exact same off literally any instance. Weird, but consistent weird.
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with HEAD + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> head(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::HEAD).with_path(path);
        return req;
    }
    /**
     * @brief Spins up a fresh POST request pointed at `path`.
     * @warning Same non-static quirk as get() — see that one for the full rundown.
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with POST + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> post(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::POST).with_path(path);
        return req;
    }
    /**
     * @brief Spins up a fresh PUT request pointed at `path`.
     * @warning Same non-static quirk as get().
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with PUT + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> put(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::PUT).with_path(path);
        return req;
    }
    /**
     * @brief Spins up a fresh DELETE request pointed at `path`. That resource's cooked once this
     * actually gets sent.
     * @warning Same non-static quirk as get().
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with DELETE + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> del(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::DELETE).with_path(path);
        return req;
    }
    /**
     * @brief Spins up a fresh PATCH request pointed at `path`.
     * @warning Same non-static quirk as get().
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with PATCH + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> patch(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::PATCH).with_path(path);
        return req;
    }
    /**
     * @brief Spins up a fresh OPTIONS request pointed at `path`. Last one of the crew, same
     * pattern all the way down.
     * @warning Same non-static quirk as get().
     * @param stream_id the stream id to tag the new request with.
     * @param path the request path/target.
     * @return a heap-allocated request already wired up with OPTIONS + `path`.
     */
    [[nodiscard]] static std::unique_ptr<IRequest> options(std::uint32_t stream_id,
                                                           std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::OPTIONS).with_path(path);
        return req;
    }

    /**
     * @brief Builder chain — sets the method from the `Method` enum. This is the type-safe way
     * to do it, prefer this one over the raw-string overload when you can.
     * @param method the HTTP method to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_method(types::Method method) && {
        set_header(types::Token::METHOD, method_str(method));
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the method from a raw string, for when you've got some
     * non-standard verb the `Method` enum doesn't cover.
     * @param method the method string to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_method(std::string_view method) && {
        set_header(types::Token::METHOD, method);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the request path. The actual destination, basically.
     * @param path the path to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_path(std::string_view path) && {
        set_header(types::Token::PATH, path);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the URI scheme (http/https/etc).
     * @param schema the scheme to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_scheme(std::string_view schema) && {
        set_header(types::Token::SCHEME, schema);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the authority (host[:port]).
     * @param authority the authority to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_authority(std::string_view authority) && {
        set_header(types::Token::AUTHORITY, authority);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets a header by string name. The general-purpose one, works for
     * literally any header.
     * @param name the header name.
     * @param value the header value.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_header(std::string_view name, std::string_view value) && {
        set_header(name, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets a header by interned `Token`, the faster path for whatever's
     * already got a known token instead of hashing a raw string every time.
     * @param token the header's token.
     * @param value the header value.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_header(types::Token token, std::string_view value) && {
        set_header(token, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — drops a header by name if it's set. No-op if it isn't, no L for
     * removing something that was never there.
     * @param name the header name to remove.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&without_header(std::string_view name) && noexcept {
        remove_header(name);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — removes then re-sets a header by string name, swapping the value
     * out clean instead of stacking a dupe entry on top of the old one.
     * @param name the header name to replace.
     * @param value the new value.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&replace_header(std::string_view name, std::string_view value) && noexcept {
        remove_header(name);
        set_header(name, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — removes then re-sets a header by `Token`, same energy as the
     * string-name overload above.
     * @param token the header's token to replace.
     * @param value the new value.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&replace_header(types::Token token, std::string_view value) && noexcept {
        remove_header(token);
        set_header(token, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — wipes every header off this request. Clean slate.
     * @note This one calls something named `clear_headers()` from inside itself, but that's not
     * self-recursion blowing your stack — ref-qualifier overload resolution routes the call to
     * the separate `&`-qualified virtual `clear_headers()` sitting down in the interface
     * section. Sneaky little gotcha if you're not watching the ref-qualifiers closely, don't get
     * caught out by it.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&clear_headers() && noexcept {
        clear_headers();
        return std::move(*this);
    }

    /**
     * @brief Builder chain — url-encodes and appends a `key=value` query param onto the path.
     * Stack as many of these as you need, they just keep piling on.
     * @param key the query key, gets url-encoded.
     * @param value the query value, gets url-encoded.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_query(std::string_view key, std::string_view value) && {
        add_query(key, value);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets `Authorization: Bearer <token>`. Standard bearer-token motion.
     * @param token the bearer token.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_bearer_auth(std::string_view token) && {
        set_header(types::Token::AUTHORIZATION, "Bearer " + std::string(token));
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets `Authorization: Basic <base64(user:password)>`.
     * @param user the username.
     * @param password the password.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_basic_auth(std::string_view user, std::string_view password) && {
        set_header(types::Token::AUTHORIZATION,
                   "Basic " + utils::encode::base64_encode(std::string(user) + ":" +
                                                           std::string(password)));
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Content-Type header.
     * @param mime the mime type to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_content_type(std::string_view mime) && {
        set_header(types::Token::CONTENT_TYPE, mime);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the Accept header.
     * @param mime the mime type to accept.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_accept(std::string_view mime) && {
        set_header(types::Token::ACCEPT, mime);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the User-Agent header.
     * @param user the user-agent string.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_user_agent(std::string_view user) && {
        set_header(types::Token::USER_AGENT, user);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the request timeout, so it doesn't just hang out forever
     * waiting on a dead connection.
     * @param timeout how long before this request should be considered timed out.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_timeout(std::chrono::milliseconds timeout) && noexcept {
        set_timeout(timeout);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — toggles whether response decompression should be skipped.
     * @param disable true to disable decompression (default), false to leave it enabled.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_no_decompress(bool disable = true) && noexcept {
        set_no_decompress(disable);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the target address for this request.
     * @param addr the address to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_addr(std::string_view addr) && noexcept {
        set_addr(addr);
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the stream id.
     * @param stream_id the stream id to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    IRequest &&with_stream_id(std::uint32_t stream_id) && noexcept {
        set_stream_id(stream_id);
        return std::move(*this);
    }

    /**
     * @brief Terminal builder call — closes out the chain, nothing else happens here, just hands
     * back what you already built.
     * @return `*this`, moved out to the caller as the finished request.
     */
    [[nodiscard]] IRequest &&build() && { return std::move(*this); }

    /**
     * @brief In-place (lvalue) version of with_method(Method) — mutates `*this` directly
     * instead of chaining through a moved rvalue. Same result, different vibe.
     * @param method the HTTP method to set.
     */
    void add_method(types::Method method) & noexcept {
        set_header(types::Token::METHOD, method_str(method));
    }

    /**
     * @brief In-place version of with_method(string_view).
     * @param method the method string to set.
     */
    void add_method(std::string_view method) & noexcept {
        set_header(types::Token::METHOD, method);
    }

    /**
     * @brief In-place version of with_path().
     * @param path the path to set.
     */
    void add_path(std::string_view path) & noexcept { set_header(types::Token::PATH, path); }

    /**
     * @brief In-place version of with_scheme().
     * @param schema the scheme to set.
     */
    void add_scheme(std::string_view schema) & noexcept {
        set_header(types::Token::SCHEME, schema);
    }

    /**
     * @brief In-place version of with_authority().
     * @param authority the authority to set.
     */
    void add_authority(std::string_view authority) & noexcept {
        set_header(types::Token::AUTHORITY, authority);
    }

    /**
     * @brief Appends a url-encoded `key=value` pair onto the existing path, picking `?` or `&`
     * depending on whether a query string's already started — smart enough not to double up on
     * separators. Empty path just becomes `/` first so there's always something to append onto.
     * @param key the query key, gets url-encoded.
     * @param value the query value, gets url-encoded.
     */
    void add_query(std::string_view key, std::string_view value) & noexcept {
        auto path_field = get_path();
        std::string new_path;

        // No path set yet — start from root so there's always something to append the query onto.
        if (path_field.empty()) {
            new_path = "/";
        } else {
            new_path = std::string(path_field);
        }

        // Bet — a query string already going means the next param needs `&`, otherwise this is
        // the first one so `?` kicks it off.
        new_path += new_path.contains('?') ? '&' : '?';
        new_path += utils::encode::url_encode(key);
        new_path += '=';
        new_path += utils::encode::url_encode(value);

        // Push the rebuilt path (query and all) back as the header — there's no separate
        // query-string field, it all lives folded into :path.
        set_header(types::Token::PATH, new_path);
    }

    /**
     * @brief In-place version of with_bearer_auth().
     * @param token the bearer token.
     */
    void add_bearer_auth(std::string_view token) & noexcept {
        set_header(types::Token::AUTHORIZATION, "Bearer " + std::string(token));
    }

    /**
     * @brief In-place version of with_basic_auth().
     * @param user the username.
     * @param password the password.
     */
    void add_basic_auth(std::string_view user, std::string_view password) & noexcept {
        set_header(types::Token::AUTHORIZATION,
                   "Basic " + utils::encode::base64_encode(std::string(user) + ":" +
                                                           std::string(password)));
    }

    /**
     * @brief In-place version of with_content_type().
     * @param mime the mime type to set.
     */
    void add_content_type(std::string_view mime) & noexcept {
        set_header(types::Token::CONTENT_TYPE, mime);
    }

    /**
     * @brief In-place version of with_accept().
     * @param mime the mime type to accept.
     */
    void add_accept(std::string_view mime) & noexcept { set_header(types::Token::ACCEPT, mime); }

    /**
     * @brief In-place version of with_user_agent().
     * @param user the user-agent string.
     */
    void add_user_agent(std::string_view user) & noexcept {
        set_header(types::Token::USER_AGENT, user);
    }

    /**
     * @brief In-place version of with_timeout().
     * @param timeout the timeout to set.
     */
    void add_timeout(std::chrono::milliseconds timeout) & noexcept { set_timeout(timeout); }

    /**
     * @brief In-place version of with_no_decompress().
     * @param disable true to disable decompression (default), false to leave it enabled.
     */
    void add_no_decompress(bool disable = true) & noexcept { set_no_decompress(disable); }

    /**
     * @brief Supposed to be the in-place version of with_addr() — but peep the body, it's
     * calling `set_header` with the `AUTHORIZATION` token, not an address-specific one. Straight
     * cooked.
     * @warning This is a real bug, not vibes. Reads like it got copy-pasted from
     * add_bearer_auth()/add_basic_auth() right above and somebody forgot to swap the call over
     * to set_addr(). Calling this stomps the Authorization header with `addr` instead of
     * actually setting the target address — meanwhile with_addr() up in the builder-chain
     * section does it correctly (calls set_addr() like it should), so these two "same method,
     * different ref-qualifier" siblings quietly do NOT do the same thing. That's an L waiting to
     * happen for whoever calls this expecting address-setting behavior. Flagging it here since
     * this pass is comment-only — not touching the logic, but don't sleep on this one.
     * @param addr gets written into the Authorization header (see warning above, that's the bug).
     */
    void add_addr(std::string_view addr) & noexcept {
        set_header(types::Token::AUTHORIZATION, addr);
    }

    /**
     * @brief In-place version of with_stream_id().
     * @param stream_id the stream id to set.
     */
    void add_stream_id(std::uint32_t stream_id) & noexcept { set_stream_id(stream_id); }

    /**
     * @brief In-place version of with_header() — takes either a name or a `Token`, whichever's
     * on hand.
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
     * @brief Sets the stream id directly on the member, no header round-trip involved, straight
     * to the point.
     * @param stream_id the stream id to store.
     */
    void set_stream_id(std::uint32_t stream_id) & noexcept { m_stream_id = stream_id; }
    /**
     * @brief Sets the timeout directly on the member.
     * @param timeout the timeout to store.
     */
    void set_timeout(std::chrono::milliseconds timeout) & noexcept { m_timeout = timeout; }

    /**
     * @brief Grabs the stream id.
     * @return the stream id this request is tagged with.
     */
    [[nodiscard]] std::uint32_t get_stream_id() const noexcept { return m_stream_id; }
    /**
     * @brief Grabs the configured timeout.
     * @return the timeout for this request.
     */
    [[nodiscard]] const std::chrono::milliseconds &get_timeout() const noexcept {
        return m_timeout;
    }

    // --- Virtual Interface ---

    /**
     * @brief Sets a header by name or `Token` — this is the real customization point, the one
     * that every `with_header`/`add_header` overload up above ultimately funnels down into.
     * @warning Base impl is just `std::abort()`, no cap, straight crash. This MUST be overridden
     * by every concrete request type or the entire header-setting API is a hard crash waiting to
     * happen the second anyone calls it. No silent no-op fallback to save you here — forget to
     * override this and it's cooked, full stop, program's going down.
     * @param name_or_token the header name, or its interned token.
     * @param value the header value.
     */
    virtual void set_header(std::variant<std::string_view, types::Token> name_or_token,
                            std::string_view value) & {
        std::abort();
    }

    /**
     * @brief Removes a header by name or `Token`.
     * @warning Base impl aborts — mandatory override, same deal as set_header() right above, no
     * exceptions.
     * @param name the header name (or token) to remove.
     */
    virtual void remove_header(std::variant<std::string_view, types::Token> name) & {
        std::abort();
    }

    /**
     * @brief Looks up a header's value by name or `Token`.
     * @warning Base impl aborts — mandatory override, same story as the rest of this section.
     * @param name_or_token the header name, or its interned token.
     * @return the header's value if it's set.
     */
    [[nodiscard]] virtual std::string_view
    find_header(std::variant<std::string_view, types::Token> name_or_token) const noexcept {
        std::abort();
    }

    /**
     * @brief Wipes every header off this request. This is the real `&`-qualified impl that the
     * rvalue `with`-style clear_headers() wrapper up in the builder section actually routes
     * into, not some coincidence of naming.
     * @warning Base impl aborts — mandatory override, same as every other method down here.
     */
    virtual void clear_headers() & noexcept { std::abort(); }


    /**
     * @brief Toggles whether response decompression should be skipped for this request.
     * @warning Base impl aborts — mandatory override.
     * @param disable true to disable decompression, false to leave it on.
     */
    virtual void set_no_decompress(bool disable) & noexcept { std::abort(); }
    /**
     * @brief Sets the target address for this request.
     * @warning Base impl aborts — mandatory override.
     * @param addr the address to set.
     */
    virtual void set_addr(std::string_view addr) & noexcept { std::abort(); }

    /**
     * @brief Sets the request body, taking full ownership of `body`.
     * @warning Base impl aborts — mandatory override, and heads up, every other set_body
     * overload floating around this class (the range-based template right below included) all
     * funnel down into this exact one eventually. Override this one and the rest just work.
     * @param body the bytes to install as the request body.
     */
    // FIXME(clang-tidy): cppcoreguidelines-rvalue-reference-param-not-moved — base impl aborts and
    // never touches `body`; every override's signature must match this one exactly for dispatch,
    // so the param can't be dropped or changed.
    virtual void set_body(std::vector<std::byte> &&body) & { std::abort(); }  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved) — signature must match every override for virtual dispatch

    /**
     * @brief Sets the request body from any input range of `std::byte` — materializes the range
     * into a real `std::vector<std::byte>` and forwards straight to the virtual set_body()
     * overload right above. Convenience wrapper, that's it.
     * @tparam R an input range whose value type is `std::byte` (and isn't already a
     * `std::vector<std::byte>` — that case is handled directly by the virtual overload, no need
     * to route it through here too).
     * @param body_range the byte range to copy into the body.
     */
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte> &&
                 (!std::same_as<std::remove_cvref_t<R>, std::vector<std::byte>>)
    void set_body(R &&body_range) & {
        set_body(std::ranges::to<std::vector<std::byte>>(std::forward<R>(body_range)));
    }

    /**
     * @brief Grabs the method header. GET, POST, whatever got set.
     * @warning Base impl aborts — same mandatory-override deal as the whole rest of this
     * section, not repeating the full speech for every single getter but consider it said.
     * @return the method string.
     */
    [[nodiscard]] virtual std::string_view get_method() const noexcept { std::abort(); }
    /**
     * @brief Grabs the path header.
     * @warning Base impl aborts — mandatory override, no exceptions.
     * @return the path string.
     */
    [[nodiscard]] virtual std::string_view get_path() const noexcept { std::abort(); }
    /**
     * @brief Grabs the host.
     * @warning Base impl aborts — mandatory override.
     * @return the host string.
     */
    [[nodiscard]] virtual std::string_view get_host() const noexcept { std::abort(); }
    /**
     * @brief Grabs the scheme header.
     * @warning Base impl aborts — mandatory override.
     * @return the scheme string.
     */
    [[nodiscard]] virtual std::string_view get_scheme() const noexcept { std::abort(); }
    /**
     * @brief Grabs the authority header.
     * @warning Base impl aborts — mandatory override, still.
     * @return the authority string.
     */
    [[nodiscard]] virtual std::string_view get_authority() const noexcept { std::abort(); }
    /**
     * @brief Grabs the content-type header.
     * @warning Base impl aborts — mandatory override.
     * @return the content-type string.
     */
    [[nodiscard]] virtual std::string_view get_content_type() const noexcept { std::abort(); }
    /**
     * @brief Grabs the accept header.
     * @warning Base impl aborts — mandatory override.
     * @return the accept string.
     */
    [[nodiscard]] virtual std::string_view get_accept() const noexcept { std::abort(); }
    /**
     * @brief Grabs the user-agent header.
     * @warning Base impl aborts — mandatory override, at this point you know the drill.
     * @return the user-agent string.
     */
    [[nodiscard]] virtual std::string_view get_user_agent() const noexcept { std::abort(); }
    /**
     * @brief Grabs the authorization header. Handle with care, that's sensitive stuff.
     * @warning Base impl aborts — mandatory override.
     * @return the authorization string.
     */
    [[nodiscard]] virtual std::string_view get_authorization() const noexcept { std::abort(); }
    /**
     * @brief Grabs a mutable view over the request body, so callers can actually poke at it.
     * @warning Base impl aborts — mandatory override.
     * @return a mutable `BufferView` over the body bytes.
     */
    [[nodiscard]] virtual utils::buffering::BufferView &get_body() noexcept { std::abort(); }
    /**
     * @brief Const overload — grabs a read-only view over the request body, look but don't touch.
     * @warning Base impl aborts — mandatory override.
     * @return a read-only `BufferView` over the body bytes.
     */
    [[nodiscard]] virtual const utils::buffering::BufferView &get_body() const noexcept {
        std::abort();
    }
    /**
     * @brief Grabs every header currently set on this request, the whole collection in one go.
     * @warning Base impl aborts — mandatory override, last one in this section, you made it.
     * @return all headers as a vector of `HeaderEntry` (static or dynamic flavored).
     */
    [[nodiscard]] virtual std::vector<HeaderEntry> get_headers() const noexcept { std::abort(); }

  private:
    std::uint32_t m_stream_id;
    std::chrono::milliseconds m_timeout;
}; // namespace interfaces::io

} // namespace interfaces::io
