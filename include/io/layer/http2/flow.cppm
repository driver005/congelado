module;
#include <stdexcept>
export module io_layer_http2:flow;

import std;
import shared;
import core_events;
import core_logger;
import core_contract;
import :extension;
import :handshake;
import :session;
import :executor;
import :request;

export namespace io::layer::http2 {

class ServerFlow {
  public:
    /**
     * @brief Wires up a fresh server-side flow — spins up the `Session` and hands the
     * `Handshake` a submitter callback that routes straight back into `m_session.send_node()`.
     * Handshake starts life not-completed, obviously, nothing's happened yet.
     * @param send callback the session uses to push bytes out to the transport.
     * @param close callback the session calls to tear the connection down.
     * @param extension_registry the process's one `HttpExtensionRegistry`, forwarded straight
     * into the `Session` ctor — see that ctor's own doc comment for why it's a required
     * reference rather than something `Session` resolves ambiently.
     * @param dispatch request/response dispatch hook, forwarded into the `Session`.
     */
    ServerFlow(::shared::SendCallback send, ::shared::CloseCallback close,
               HttpExtensionRegistry &extension_registry, core::contract::ContractGroup<> &group,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(send), std::move(close), extension_registry, Role::SERVER,
                    std::move(dispatch)},
          m_executor{m_session},
          m_handshake{m_session.get_local_settings(), [this](utils::buffering::BufferNode &&node) {
                          m_session.send_node(std::move(node));
                      }} {
        // Register the frame executor as its own self-rescheduling contract — the socket read
        // callback (see on_read()) hands bytes to it instead of parsing inline. SCHEDULED so it
        // starts running immediately and keeps draining regardless of when reads land.
        m_executor_contract.emplace(
            m_executor.create(group, core::contract::ContractState::SCHEDULED));
    }


    /**
     * @brief Tears this connection's session down — sends GOAWAY and, unless `graceful`, invokes
     * the close callback the transport wired up at accept time (which is what actually closes the
     * underlying socket). Graceful leaves the socket open so the sender can flush first.
     * @param code the GOAWAY error code to report to the peer; defaults to a clean shutdown.
     * @param graceful when true, skip the socket close so the transport stays open.
     */
    void close(error::http::Http2ErrorCode code = error::http::Http2ErrorCode::NO_ERROR_CODE,
               bool graceful = false) {
        if (m_closed) {
            return;
        }
        m_closed = true;
        m_session.close(code, 0, graceful);
        // Ask the frame executor to drain any final frames, then release its own contract once the
        // session's fully idle — instead of yanking it out immediately. is_idle() waits on it.
        m_executor.mark_close();
    }

    /**
     * @brief Builds the read callback the transport layer calls whenever bytes come in off the
     * wire. Runs the connection-preface handshake first (once, gated by
     * `m_handshake_completed`), and only after that's locked in does it start forwarding real
     * frame bytes down into `m_session.receive()`.
     * @warning A bad preface closes the connection with PROTOCOL_ERROR and just returns —
     * whatever came in after the bad preface never reaches the session. Same energy for a
     * still-incomplete preface: it bails quietly and waits for more bytes next call, no L
     * logged, that's expected steady-state not an error.
                    // Bad preface — L, kill the connection and don't touch whatever's left in view.
                    core::logger::error("http2/server/flow", "invalid preface");
                    core::events::publish("http2.flow.invalid_preface");
                    close(error::http::Http2ErrorCode::PROTOCOL_ERROR);
                    return;
                } else {
                    // Still incomplete, just need more bytes — bail quietly and wait for next call.
                    return;
                }
            }
            // Handshake's locked in (or already was) — anything left in view is real frame
            // bytes, hand it off to the session.
            if (!view.empty()) {
                core::logger::debug("http2/server/flow", "dispatch to session");
                m_session.receive(view);
            }
        };
    }

    /**
     * @brief Checks whether this connection has nothing left to send and no active streams.
     * @return true if the connection is finished and can be closed.
     */
    [[nodiscard]] bool is_idle() noexcept { return m_session.is_idle(); }

  private:
    Session m_session;
    Handshake<true> m_handshake;
    bool m_handshake_completed{false};
    bool m_closed{false};
};

class ClientFlow {
  public:
    using OnConnectCallback = std::function<::shared::ReadCallback()>;

    /**
     * @brief Wires up a fresh client-side flow, same deal as `ServerFlow`'s ctor — `Session`
     * gets built, `Handshake` gets a submitter callback that routes back through
     * `m_session.send_node()`.
     * @param on_send callback the session uses to push bytes out to the transport.
     * @param close callback the session calls to tear the connection down.
     * @param extension_registry the process's one `HttpExtensionRegistry`, forwarded straight
     * into the `Session` ctor.
     * @param dispatch request/response dispatch hook, forwarded into the `Session`.
     */
    ClientFlow(::shared::SendCallback on_send, ::shared::CloseCallback close,
               HttpExtensionRegistry &extension_registry,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(on_send), std::move(close), extension_registry, std::move(dispatch)},
          m_handshake{m_session.get_local_settings(), [this](utils::buffering::BufferNode &&node) {
                          m_session.send_node(std::move(node));
                      }} {}

    /**
     * @brief Builds the on-connect callback — fires the client handshake (unconditionally, no
     * preface-checking needed on this side since the client is the one sending it) and then
     * returns the read callback that'll handle every byte coming back after that point.
     * @note Unlike `ServerFlow::on_read()`, there's no completed-flag gating here — client-side
     * handshake is fire-and-forget synchronous before the read callback even gets constructed.
     * @return a callback that, once invoked, runs the handshake and hands back the steady-state
     * read callback.
     */
    OnConnectCallback on_connect() {
        return [this]() {
            core::logger::debug("http2/client/flow", "handshake");

            // Client sends the preface, no waiting on the peer to confirm — synchronous and done.
            m_handshake.process(m_session.get_extension_registry());

            core::logger::debug("http2/client/flow", "handshake ok");

            // Steady-state read callback for everything that comes back after the handshake.
            return [this](utils::buffering::BufferReader &view) {
                core::logger::debug("http2/client/flow", "rx {} bytes", view.size());
                if (!view.empty()) {
     * @brief Wires up a fresh client-side flow, same deal as `ServerFlow`'s ctor — `Session`
     * gets built, `Handshake` gets a submitter callback that routes back through
     * `m_session.send_node()`.
     * @param on_send callback the session uses to push bytes out to the transport.
     * @param close callback the session calls to tear the connection down.
     * @param extension_registry the process's one `HttpExtensionRegistry`, forwarded straight
     * into the `Session` ctor.
     * @param dispatch request/response dispatch hook, forwarded into the `Session`.
     */
    ClientFlow(::shared::SendCallback on_send, ::shared::CloseCallback close,
               HttpExtensionRegistry &extension_registry, core::contract::ContractGroup<> &group,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(on_send), std::move(close), extension_registry, Role::CLIENT,
                    std::move(dispatch)},
          m_executor{m_session},
          m_handshake{m_session.get_local_settings(), [this](utils::buffering::BufferNode &&node) {
                          m_session.send_node(std::move(node));
                      }} {
        // Same self-rescheduling frame executor as ServerFlow — the socket read callback (built in
        // on_connect()) hands response bytes to it instead of parsing inline.
        m_executor_contract.emplace(
            m_executor.create(group, core::contract::ContractState::SCHEDULED));
    }

    /// @brief Releases the frame executor's contract so it stops rescheduling before this flow (and
    /// the executor it owns) is torn down.
    ~ClientFlow() {
        if (m_executor_contract.has_value()) {
            m_executor_contract->release();
        }
    }

    ClientFlow(const ClientFlow &) = delete;
    ClientFlow &operator=(const ClientFlow &) = delete;
    ClientFlow(ClientFlow &&) = delete;
    ClientFlow &operator=(ClientFlow &&) = delete;

    /**
     * @brief Builds the on-connect callback — fires the client handshake (unconditionally, no
     * preface-checking needed on this side since the client is the one sending it) and then
     * returns the read callback that'll handle every byte coming back after that point.
     * @note Unlike `ServerFlow::on_read()`, there's no completed-flag gating here — client-side
     * handshake is fire-and-forget synchronous before the read callback even gets constructed.
     * @return a callback that, once invoked, runs the handshake and hands back the steady-state
     * read callback.
     */
    OnConnectCallback on_connect() {
        return [this]() {
            core::logger::debug("http2/client/flow", "handshake");

            // Client sends the preface, no waiting on the peer to confirm — synchronous and done.
            m_handshake.process(m_session.get_extension_registry());

            core::logger::debug("http2/client/flow", "handshake ok");

            // Steady-state read callback for everything that comes back after the handshake.
            return [this](utils::buffering::BufferReader &view) {
                core::logger::debug("http2/client/flow", "rx {} bytes", view.size());
                if (!view.empty()) {
                    core::logger::debug("http2/client/flow", "feed executor");
                    m_executor.feed(view);
                    view.consume(view.size());
                }
            };
        };
    }

    /**
     * @brief Hands a request off to the session for framing and dispatch. Thin wrapper, bet —
     * all the actual stream-assignment and HEADERS/DATA framing motion lives in
     * `Session::send()`.
     * @param request the request to send. Session tags it with a fresh client stream id.
     * @return the stream id the session assigned — the response-correlation key.
     */
    std::uint32_t sender(HttpRequest &request) { return m_session.send(request); }

    /**
     * @brief Whether this connection has nothing left to send and no active streams — and the
     * frame executor has finished draining and released its contract.
     * @return true if the session is idle and the executor's done.
     */
    [[nodiscard]] bool is_idle() noexcept {
        // Idle only once the session has no active streams AND the frame executor has finished
        // draining and released its contract (see SessionExecutor::request_close/on_execute).
        auto done = [](const std::optional<core::contract::Contract<>> &contract) {
            return !contract.has_value() || contract->is_released() || contract->is_idle();
        };
        return m_session.is_idle() && done(m_executor_contract);
    }


  private:
    Session m_session;
    SessionExecutor m_executor;
    Handshake<false> m_handshake;
    std::optional<core::contract::Contract<>> m_executor_contract;
};

} // namespace io::layer::http2
