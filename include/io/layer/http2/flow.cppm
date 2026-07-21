module;
#include <stdexcept>
export module io_layer_http2:flow;

import std;
import shared;
import core_logger;
import :handshake;
import :session;
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
     * @param dispatch request/response dispatch hook, forwarded into the `Session`.
     */
    ServerFlow(::shared::SendCallback send, ::shared::CloseCallback close,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(send), std::move(close), std::move(dispatch)},
          m_handshake{m_session.get_local_settings(),
                      [this](utils::buffering::BufferNode &&node) {
                          m_session.send_node(std::move(node));
                      }} {}

    /**
     * @brief Builds the read callback the transport layer calls whenever bytes come in off the
     * wire. Runs the connection-preface handshake first (once, gated by
     * `m_handshake_completed`), and only after that's locked in does it start forwarding real
     * frame bytes down into `m_session.receive()`.
     * @warning A bad preface closes the connection with PROTOCOL_ERROR and just returns —
     * whatever came in after the bad preface never reaches the session. Same energy for a
     * still-incomplete preface: it bails quietly and waits for more bytes next call, no L
     * logged, that's expected steady-state not an error.
     * @return the read callback, bound to `this`, ready to hand off to the transport.
     */
    ::shared::ReadCallback on_read() {
        return [this](utils::buffering::BufferReader &view) {
            core::logger::debug("http2/server/flow", "rx {} bytes", view.size());
            // Preface handshake hasn't wrapped up yet — feed these bytes into it instead of
            // treating them as frame data.
            if (!m_handshake_completed) {
                core::logger::debug("http2/server/flow", "handshake");
                auto result = m_handshake.process(view);
                if (result == HandshakeState::COMPLETED) {
                    // Preface matched, flip the flag so future calls skip straight to framing.
                    core::logger::debug("http2/server/flow", "handshake ok");
                    m_handshake_completed = true;
                } else if (result == HandshakeState::PREFACE_ERROR) {
                    // Bad preface — L, kill the connection and don't touch whatever's left in view.
                    core::logger::error("http2/server/flow", "invalid preface");
                    m_session.close(error::http::Http2ErrorCode::PROTOCOL_ERROR);
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

  private:
    Session m_session;
    Handshake<true> m_handshake;
    bool m_handshake_completed{false};
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
     * @param dispatch request/response dispatch hook, forwarded into the `Session`.
     */
    ClientFlow(::shared::SendCallback on_send, ::shared::CloseCallback close,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(on_send), std::move(close), std::move(dispatch)},
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
            m_handshake.process();

            core::logger::debug("http2/client/flow", "handshake ok");

            // Steady-state read callback for everything that comes back after the handshake.
            return [this](utils::buffering::BufferReader &view) {
                core::logger::debug("http2/client/flow", "rx {} bytes", view.size());
                if (!view.empty()) {
                    core::logger::debug("http2/client/flow", "dispatch to session");
                    m_session.receive(view);
                }
            };
        };
    }

    /**
     * @brief Hands a request off to the session for framing and dispatch. Thin wrapper, bet —
     * all the actual stream-assignment and HEADERS/DATA framing motion lives in
     * `Session::send()`.
     * @param request the request to send. Session tags it with a fresh client stream id.
     */
    void sender(HttpRequest &request) { m_session.send(request); }


  private:
    Session m_session;
    Handshake<false> m_handshake;
};

} // namespace io::layer::http2
