export module core_client;

import std;
import interfaces;

export namespace core::client {

class Client {
  public:
    /**
     * @brief Deleted — no bare Client allowed, every instance has to come from one of the
     * static factories (get()/post()/etc) below so method+path are always set together.
     */
    Client() = delete;
    /**
     * @brief Defaulted dtor, nothing extra to tear down here.
     */
    ~Client() = default;

    /**
     * @brief Deleted — Client holds a `reference_wrapper` onto the runtime IClient, copying
     * that reference around is straight up the wrong move, so copying's off the table.
     */
    Client(const Client &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor right above.
     */
    Client &operator=(const Client &) = delete;

    /**
     * @brief Defaulted move ctor — moving a Client around is all good, it's only copying
     * that's banned.
     */
    Client(Client &&) = default;
    /**
     * @brief Defaulted move assign, same deal as the move ctor.
     */
    Client &operator=(Client &&) = default;

    /**
     * @brief Spins up a fresh GET Client pointed at `path`. Bet, straightforward one.
     * @param path the request path/target.
     * @return a Client pre-loaded with the GET method and `path`.
     */
    static Client get(std::string_view path) { return {"GET", path}; }
    /**
     * @brief Spins up a fresh HEAD Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the HEAD method and `path`.
     */
    static Client head(std::string_view path) { return {"HEAD", path}; }
    /**
     * @brief Spins up a fresh POST Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the POST method and `path`.
     */
    static Client post(std::string_view path) { return {"POST", path}; }
    /**
     * @brief Spins up a fresh PUT Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the PUT method and `path`.
     */
    static Client put(std::string_view path) { return {"PUT", path}; }
    /**
     * @brief Spins up a fresh DELETE Client pointed at `path`. That resource's cooked once
     * this actually gets sent.
     * @param path the request path/target.
     * @return a Client pre-loaded with the DELETE method and `path`.
     */
    static Client del(std::string_view path) { return {"DELETE", path}; }
    /**
     * @brief Spins up a fresh PATCH Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the PATCH method and `path`.
     */
    static Client patch(std::string_view path) { return {"PATCH", path}; }
    /**
     * @brief Spins up a fresh OPTIONS Client pointed at `path`. Last one of the crew, same
     * pattern all the way down.
     * @param path the request path/target.
     * @return a Client pre-loaded with the OPTIONS method and `path`.
     */
    static Client options(std::string_view path) { return {"OPTIONS", path}; }

    /**
     * @brief Builder chain — wires this Client up to the runtime IClient that'll actually ship
     * the request out.
     * @param client the runtime client to bind, kept as a reference so no ownership motion
     * happens here.
     * @return `*this`, moved, so the chain keeps going.
     */
    Client &&with_runtime(interfaces::IClient &client) && {
        m_client = client;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the callback that fires whenever data comes back in on this
     * request.
     * @param func the receive-dispatch callback to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    Client &&on_receive(interfaces::io::ReceiveDispatchFn &&func) && {
        m_receive_dispatch_fn = std::move(func);
        return std::move(*this);
    }

    /**
     * @brief In-place version of with_runtime() — mutates `*this` directly instead of chaining
     * through a moved rvalue.
     * @param client the runtime client to bind.
     */
    void set_runtime(interfaces::IClient &client) { m_client = client; }

    /**
     * @brief In-place version of on_receive().
     * @param func the receive-dispatch callback to install.
     */
    void set_on_receive(interfaces::io::ReceiveDispatchFn &&func) {
        m_receive_dispatch_fn = std::move(func);
    }

    /**
     * @brief Adds a header straight onto the underlying base request.
     * @param key the header name.
     * @param value the header value.
     */
    void add_header(std::string_view key, std::string_view value) {
        m_base_request.add_header(key, value);
    }

    // TODO: add_body has to append to utils::buffering::BuggerView via get_body()
    /**
     * @brief Supposed to append `body` onto the request body.
     * @warning Straight up empty right now — see the TODO right above, this is a stub that
     * does nothing. Calling it is a silent no-op, not an error, so don't expect your body to
     * actually show up on send(). No cap, this one's just not built yet.
     * @param body the bytes meant to get appended, once this actually does something.
     */
    void add_body(std::string_view body) {}


    /**
     * @brief Fires the request off through whatever runtime client got wired in via
     * with_runtime()/set_runtime().
     * @throws std::runtime_error if no runtime client was ever set — this throws loud instead
     * of quietly no-op'ing, so forgetting with_runtime() gets caught right away.
     */
    void send() {
        // Guard clause — no runtime wired in means there's nowhere to actually ship this to.
        if (!m_client.has_value()) {
            throw std::runtime_error("Please set runtime first");
        }
        // Runtime's set, bet — hand the base request off to it.
        auto &client = m_client.value().get();
        client.send(m_base_request);
    }


  private:
    /**
     * @brief Real ctor behind every static factory — stamps method and path onto the base
     * request.
     * @warning `m_base_request.add_header("authority", m_path)` sets the *authority* header to
     * the path value, not an actual host/authority. Looks like a mix-up (authority should be
     * host[:port], not the request path) but that's the existing behavior — flagging it here
     * since this pass is comment-only, not touching the logic.
     * @param method the HTTP method string (GET/POST/etc) to store as the "method" header.
     * @param path the request path/target, stored both as `m_path` and (per the warning above)
     * as the "authority" header.
     */
    Client(std::string_view method, std::string_view path)
        : m_path{path} {
        // Stamp the method header first...
        m_base_request.add_header("method", method);
        // ...then the (mislabeled, see warning above) authority header.
        m_base_request.add_header("authority", m_path);
    }

    std::string m_path;
    interfaces::io::IRequest m_base_request;
    interfaces::io::ReceiveDispatchFn m_receive_dispatch_fn;
    std::optional<std::reference_wrapper<interfaces::IClient>> m_client;
};

} // namespace core::client
