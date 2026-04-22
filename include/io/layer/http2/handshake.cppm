export module io_layer_http2:handshake;

import std;
import io_base_buffering;
import io_layer_shared;
import :consts;
import :settings;

export namespace io::layer::http2 {

enum class HandshakeState { AwaitingPreface, PrefaceReceived, PrefaceError, Completed };

template <bool IsServer = true>
class Handshake {
  public:
    Handshake(Settings &settings, shared::SendCallback submiter)
        : m_local_settings{settings}, m_submiter{std::move(submiter)}, m_sent_settings{false} {}


    HandshakeState process(base::buffering::BufferView &view) {
        if constexpr (IsServer) {
            send_handshake();

            return is_valid_preface(view);
        } else {
            send_handshake();

            return HandshakeState::Completed;
        }
    }

  private:
    HandshakeState is_valid_preface(base::buffering::BufferView &view) const {
        const auto &preface = HTTP2_CONNECTION_PREFACE;
        if (view.size() < preface.size()) {
            return HandshakeState::AwaitingPreface;
        }

        if (const auto *ptr = view.peek_contiguous(preface.size())) {
            if (std::memcmp(ptr, preface.data(), preface.size()) == 0) {
                return HandshakeState::Completed;
            } else {
                return HandshakeState::PrefaceError;
            }
        }

        std::size_t offset = 0;
        for (const auto &byte : view) {
            if (byte != preface[offset++]) {
                return HandshakeState::PrefaceError;
            }
            if (offset == preface.size()) {
                return HandshakeState::Completed;
            }
        }

        return HandshakeState::PrefaceError;
    }
    void send_handshake() {
        if (m_sent_settings) {
            return;
        }

        std::vector<std::byte> payload;

        m_local_settings.get().encode(payload);

        auto frame = Frame<shared_layer::FrameRole::Sender>{}
                         .add_header(FrameHeader{}
                                         .add_length(static_cast<std::uint32_t>(payload.size()))
                                         .add_type(shared_layer::FrameType::SETTINGS)
                                         .add_flags(0)
                                         .add_stream_id(0))
                         .add_payload(payload)
                         .build();

        if constexpr (IsServer) {
            base::buffering::BufferNode node{HTTP2_CONNECTION_PREFACE.data(),
                                             HTTP2_CONNECTION_PREFACE.size() + frame.get_size()};
            frame.encode(node);
            m_submiter(std::move(node));
        } else {
            base::buffering::BufferNode node{frame.get_size()};
            frame.encode(node);
            m_submiter(std::move(node));
        }
    }

    std::reference_wrapper<Settings> m_local_settings;
    shared::SendCallback m_submiter;
    bool m_sent_settings;
};
} // namespace io::layer::http2
