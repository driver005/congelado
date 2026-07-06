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
    ServerFlow(::shared::SendCallback send, ::shared::CloseCallback close,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(send), std::move(close), std::move(dispatch)},
          m_handshake{m_session.get_local_settings(),
                      [this](utils::buffering::BufferNode &&node) {
                          m_session.send_node(std::move(node));
                      }},
          m_handshake_completed{false} {}

    ::shared::ReadCallback on_read() {
        return [this](utils::buffering::BufferReader &view) {
            core::logger::debug("http2/server/flow", "rx {} bytes", view.size());
            if (!m_handshake_completed) {
                core::logger::debug("http2/server/flow", "handshake");
                auto result = m_handshake.process(view);
                if (result == HandshakeState::COMPLETED) {
                    core::logger::debug("http2/server/flow", "handshake ok");
                    m_handshake_completed = true;
                } else if (result == HandshakeState::PREFACE_ERROR) {
                    core::logger::error("http2/server/flow", "invalid preface");
                    m_session.close(error::http::Http2ErrorCode::PROTOCOL_ERROR);
                    return;
                } else {
                    return;
                }
            }
            if (view.size() > 0) {
                core::logger::debug("http2/server/flow", "dispatch to session");
                m_session.receive(view);
            }
        };
    }

  private:
    Session m_session;
    Handshake<true> m_handshake;
    bool m_handshake_completed;
};

class ClientFlow {
  public:
    using OnConnectCallback = std::function<::shared::ReadCallback()>;

    ClientFlow(::shared::SendCallback on_send, ::shared::CloseCallback close,
               interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_session{std::move(on_send), std::move(close), dispatch},
          m_handshake{m_session.get_local_settings(), [this](utils::buffering::BufferNode &&node) {
                          m_session.send_node(std::move(node));
                      }} {}

    OnConnectCallback on_connect() {
        return [this]() {
            core::logger::debug("http2/client/flow", "handshake");

            m_handshake.process();

            core::logger::debug("http2/client/flow", "handshake ok");

            return [this](utils::buffering::BufferReader &view) {
                core::logger::debug("http2/client/flow", "rx {} bytes", view.size());
                if (view.size() > 0) {
                    core::logger::debug("http2/client/flow", "dispatch to session");
                    m_session.receive(view);
                }
            };
        };
    }

    void sender(HttpRequest &request) { m_session.send(request); }


  private:
    Session m_session;
    Handshake<false> m_handshake;
};

} // namespace io::layer::http2
