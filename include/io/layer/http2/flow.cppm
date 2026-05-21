export module io_layer_http2:flow;

import std;
import shared;
import core_logger;
import :handshake;
import :session;

export namespace io::layer::http2 {

class Flow {
  public:
    Flow(::shared::SendCallback send, ::shared::CloseCallback close)
        : m_session{std::move(send), std::move(close)},
          m_handshake{m_session.get_local_settings(),
                      [this](utils::buffering::BufferNode &&node) { m_session.send_node(std::move(node)); }},
          m_handshake_completed{false} {
        core::logger::debug("Flow - HTTP/2", "Created with send and close callbacks");
    }

    ::shared::ReadCallback on_read() {
        return [this](utils::buffering::BufferReader &view) {
            core::logger::info("Flow - HTTP/2", "Received data to process, size `{}`", view.size());
            if (!m_handshake_completed) {
                core::logger::debug("Flow - HTTP/2", "Performing handshake");
                auto result = m_handshake.process(view);
                if (result == HandshakeState::COMPLETED) {
                    core::logger::info("Flow - HTTP/2", "Handshake completed successfully");
                    m_handshake_completed = true;
                } else if (result == HandshakeState::PREFACE_ERROR) {
                    core::logger::error("Flow - HTTP/2", "Handshake failed due to invalid preface");
                    m_session.close(error::http::Http2ErrorCode::PROTOCOL_ERROR);
                    return;
                } else {
                    return;
                }
            }
            if (view.size() > 0) {
                core::logger::debug("Flow - HTTP/2", "Processing received data through session");
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
