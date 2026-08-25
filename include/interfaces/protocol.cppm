module;
#include <stdexcept>
export module interfaces:protocol;

import std;
import shared;
import io_shared;
import :client;
import :io;
#ifdef CONGELADO_TEST
import boost.ut;
import utils_buffering;
#endif

export namespace interfaces {

// Dispatch function: called by the protocol layer for each fully-received request.
// Protocol implementations call this once per request/response pair.

template <typename T>
concept ServerConcept = requires(T server, shared::SendCallback &&send,
                                 shared::CloseCallback &&close, void *router_ctx) {
    { server.on_connect(std::move(send), std::move(close)) } -> std::same_as<shared::ReadCallback>;
    { server.build(router_ctx) } -> std::same_as<void>;
    { server.close() } -> std::same_as<void>;
    { server.mark_closed() } -> std::same_as<void>;
    { server.is_idle() } -> std::same_as<bool>;
};

template <typename T>
concept ClientConcept = std::constructible_from<T, io::ReceiveDispatchFn &&> &&
                        requires(T client, shared::SendCallback &&send,
                                 shared::CloseCallback &&close, void *router_ctx) {
                            {
                                client.on_connect(std::move(send), std::move(close))
                            } -> std::same_as<shared::ReadCallback>;
                            { client.on_send() } -> std::same_as<io::SendDispatchFn>;
                        };

// Interface for io-layer protocol plugins.
// Each protocol implementation controls transport binding
// (host, port, TLS, threads) and per-connection data handling.
template <ServerConcept Server>
class IProtocol {
  public:
    /**
     * @brief Virtual dtor, default's fine — protocol plugins clean up fine through the base
     * ptr, no extra teardown motion needed.
     */
    virtual ~IProtocol() = default;
    IProtocol() = default;
    IProtocol(const IProtocol &) = delete;
    IProtocol &operator=(const IProtocol &) = delete;
    IProtocol(IProtocol &&) = delete;
    IProtocol &operator=(IProtocol &&) = delete;

    /**
     * @brief Tells you which protocol this is (http/1.1, h2, ws, whatever it identifies as) —
     * basically its whole vibe in one string.
     * @return the protocol's name.
     */
    [[nodiscard]] virtual std::string_view get_protocol_name() const noexcept = 0;
    /**
     * @brief The host this protocol binds to when it stands up a server.
     * @return the bind host string.
     */
    [[nodiscard]] virtual std::string_view get_bind_host() const noexcept = 0;
    /**
     * @brief The port this protocol binds to when it stands up a server.
     * @return the bind port.
     */
    [[nodiscard]] virtual std::uint16_t get_bind_port() const noexcept = 0;
    /**
     * @brief Path to the TLS cert this protocol should serve with, if it's doing TLS at all.
     * @return the TLS cert path/content — implementer decides which flavor.
     */
    [[nodiscard]] virtual std::string_view get_tls_cert() const noexcept = 0;
    /**
     * @brief Path to the TLS key this protocol should serve with, if it's doing TLS at all.
     * @return the TLS key path/content — implementer decides which flavor.
     */
    [[nodiscard]] virtual std::string_view get_tls_key() const noexcept = 0;

    /**
     * @brief Hands back a fresh `Server` for this protocol so the caller can start accepting
     * connections through it.
     * @warning Default impl just throws — flat out, no server, no fallback. If a protocol
     * doesn't override this, asking it for a server is a straight L, not a silent no-op you can
     * quietly ignore. Get it overridden or don't call it, those are the two options.
     * @return a heap-allocated server instance for this protocol.
     * @throws std::runtime_error if the concrete protocol never overrode this — server-side
     * support just isn't implemented here, full stop.
     */
    [[nodiscard]] virtual std::unique_ptr<Server> get_server() {
        throw std::runtime_error("IServer not implemented for this protocol");
    };
    /**
     * @brief Hands back a fresh `IClient` wired to dispatch received data through the given fn.
     * @warning Same deal as get_server() right above — default impl just throws if nobody
     * bothered to override it. Don't call this on a protocol that never wired up client
     * support, that's asking for an L. The unnamed `io::ReceiveDispatchFn &&` param is the
     * dispatch fn the resulting client should call for every received request/response.
     * @return a heap-allocated client instance for this protocol.
     * @throws std::runtime_error if the concrete protocol never overrode this — client-side
     * support isn't implemented, that's the whole story.
     */
    // FIXME(clang-tidy): cppcoreguidelines-rvalue-reference-param-not-moved — default impl
    // throws and never touches `dispatch_fn`; every override's signature must match this one
    // exactly for dispatch, so the param can't be dropped or changed.
    [[nodiscard]] virtual std::unique_ptr<IClient>
    get_client(io::ReceiveDispatchFn
                   &&dispatch_fn) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved) —
                                    // signature must match every override for virtual dispatch
        throw std::runtime_error("IClient not implemented for this protocol");
    };

    // Default no-op for protocols that build internally via build().
    /**
     * @brief Wires up the dispatch fn for protocols that don't build it internally through
     * build().
     * @note Default's a genuine no-op here, and that's intentional — protocols that already
     * handle dispatch inside build() just skip overriding this, and that's the sanctioned
     * motion, not a bug you stumbled into. The unnamed `io::ReceiveDispatchFn &&` param is the
     * dispatch fn to wire in.
     */
    // FIXME(clang-tidy): cppcoreguidelines-rvalue-reference-param-not-moved — intentional
    // default no-op; every override's signature must match this one exactly for dispatch.
    virtual void set_dispatch(io::ReceiveDispatchFn &&dispatch_fn) {
    } // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved) — signature must match every
      // override for virtual dispatch
};


using HandlerFn = std::function<void(io::IRequest &, io::IResponse &, std::function<void()> send)>;

using NextFn = std::move_only_function<void(io::IRequest &, io::IResponse &,
                                                     std::function<void()> send) noexcept>;

using MiddlewareFn =
    void (*)(io::IRequest &, io::IResponse &, NextFn &&, std::function<void()> send);

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::protocol_tests {

// Satisfies ServerConcept — no real transport, just the shape IProtocol<Server> requires.
class MockServer {
  public:
    shared::ReadCallback on_connect(shared::SendCallback &&, shared::CloseCallback &&) {
        return [](utils::buffering::BufferReader &) {};
    }
    void build(void *) {}
    void close() {}
    void mark_closed() {}
    [[nodiscard]] bool is_idle() { return true; }
};

// Minimal IProtocol fixture — leaves get_server()/get_client()/set_dispatch() at their
// defaults so those default implementations can be exercised in isolation.
class MockProtocol final : public IProtocol<MockServer> {
  public:
    [[nodiscard]] std::string_view get_protocol_name() const noexcept override { return "mock"; }
    [[nodiscard]] std::string_view get_bind_host() const noexcept override { return "127.0.0.1"; }
    [[nodiscard]] std::uint16_t get_bind_port() const noexcept override { return 0; }
    [[nodiscard]] std::string_view get_tls_cert() const noexcept override { return ""; }
    [[nodiscard]] std::string_view get_tls_key() const noexcept override { return ""; }
};

using namespace boost::ut;

suite<"IProtocol defaults"> protocol_suite = [] {
    "get_server() throws when not overridden"_test = [] {
        MockProtocol protocol;
        expect(throws<std::runtime_error>([&] { std::ignore = protocol.get_server(); }));
    };

    "get_client() throws when not overridden"_test = [] {
        MockProtocol protocol;
        expect(throws<std::runtime_error>(
            [&] { std::ignore = protocol.get_client(io::ReceiveDispatchFn{}); }));
    };

    "set_dispatch() defaults to a no-op"_test = [] {
        MockProtocol protocol;
        protocol.set_dispatch(io::ReceiveDispatchFn{});
        expect(true) << "reaching here means set_dispatch() didn't throw or crash";
    };
};

} // namespace interfaces::protocol_tests
#endif
