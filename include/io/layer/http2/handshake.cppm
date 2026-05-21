module;
#include <ranges>
export module io_layer_http2:handshake;

import std;
import utils_buffering;
import io_layer_shared;
import core_logger;
import :consts;
import :settings;
import :frame;

export namespace io::layer::http2 {

enum class HandshakeState : std::uint8_t { AWAITING_PREFACE, PREFACE_RECEIVED, PREFACE_ERROR, COMPLETED };

template <bool IsServer = true>
class Handshake {
  public:
    Handshake(Settings &settings, shared::SendCallback submiter)
        : m_local_settings{settings}, m_submiter{std::move(submiter)}, m_sent_settings{false} {
        core::logger::debug("Handshake - HTTP/2", "Created with local settings and send callback");
    }

    HandshakeState process(utils::buffering::BufferReader &view) {
        core::logger::info("Handshake - HTTP/2", "Processing handshake with data of size `{}`", view.size());
        if constexpr (IsServer) {
            send_handshake();

            return is_valid_preface(view);
        } else {
            send_handshake();

            return HandshakeState::COMPLETED;
        }
    }

  private:
    HandshakeState is_valid_preface(utils::buffering::BufferReader &view) const {
        const auto &preface = HTTP2_CONNECTION_PREFACE;
        if (view.size() < preface.size()) {
            core::logger::info("Handshake - HTTP/2",
                               "Received data size `{}` is smaller than preface size `{}`, awaiting more data",
                               view.size(), preface.size());

            return HandshakeState::AWAITING_PREFACE;
        }

        if (std::ranges::equal(preface, view | std::views::take(preface.size()))) {
            core::logger::info("Handshake - HTTP/2", "Received valid preface");

            view.consume(preface.size());
            return HandshakeState::COMPLETED;
        }

        core::logger::warning("Handshake - HTTP/2", "Received invalid preface");
        return HandshakeState::PREFACE_ERROR;
    }

    void send_handshake() {
        if (m_sent_settings) {
            core::logger::debug("Handshake - HTTP/2", "Settings frame already sent, skipping handshake send");

            return;
        }

        core::logger::info("Handshake - HTTP/2", "Sending handshake settings frame");

        auto payload = std::views::empty<std::byte> | WriteSettingsAdaptor{m_local_settings.get()} |
                       std::ranges::to<std::vector<std::byte>>();

        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::SETTINGS)
                         .add_flags(0)
                         .add_stream_id(0)
                         .add_payload(payload)
                         .build();

        if constexpr (!IsServer) {
            auto size = HTTP2_CONNECTION_PREFACE.size() + frame.get_size();
            auto adaptor = WriteFrameBuilderAdaptor{std::move(frame), m_local_settings.get().max_frame_size()};
            auto node =
                std::span{HTTP2_CONNECTION_PREFACE} | adaptor | std::ranges::to<utils::buffering::BufferNode>(size);
            m_submiter(std::move(node));
        } else {
            auto size = frame.get_size();
            auto adaptor = WriteFrameBuilderAdaptor{std::move(frame), m_local_settings.get().max_frame_size()};
            auto node = std::views::empty<std::byte> | adaptor | std::ranges::to<utils::buffering::BufferNode>(size);
            m_submiter(std::move(node));
        }

        m_sent_settings = true;
    }

    std::reference_wrapper<Settings> m_local_settings;
    shared::SendCallback m_submiter;
    bool m_sent_settings;
};
} // namespace io::layer::http2
