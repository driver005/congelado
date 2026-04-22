export module io_layer_http2:flow;

import std;
import shared;
import :handshake;
import :session;

export namespace io::layer::http2 {

class Flow {
  public:
    Flow(::shared::SendCallback send, ::shared::CloseCallback close)
        : m_session{std::move(send), std::move(close)},
          m_handshake{m_session.get_local_settings(),
                      [this](base::buffering::BufferNode &&node) { m_session.send(std::move(node)); }},
          m_handshake_completed{false} {}

    ::shared::ReadCallback on_read() {
        return [this](base::buffering::BufferView view) {
            if (!m_handshake_completed) {
                auto result = m_handshake.process(view);
                if (result == HandshakeState::Completed) {
                    m_handshake_completed = true;
                } else if (result == HandshakeState::PrefaceError) {
                    m_session.close(error::http::Http2ErrorCode::PROTOCOL_ERROR);
                }
            } else {
                m_session.receive(view);
            }
        };
    }

  private:
    Session m_session;
    Handshake<> m_handshake;
    bool m_handshake_completed;
};

} // namespace io::layer::http2
