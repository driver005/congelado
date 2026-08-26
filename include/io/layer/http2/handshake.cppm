module;
#include <ranges>
export module io_layer_http2:handshake;

import std;
import utils_buffering;
import io_layer_shared;
import core_events;
import core_logger;
import :extension;
import :consts;
import :settings;
import :frame;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::layer::http2 {

enum class HandshakeState : std::uint8_t
{
    AWAITING_PREFACE,
    PREFACE_RECEIVED,
    PREFACE_ERROR,
    COMPLETED
};

template<bool IsServer = true>
class Handshake
{
public:
    /**
     * @brief Builds a handshake bound to the local settings and a submitter callback for
     * flushing the outgoing SETTINGS frame (and, client-side, the connection preface).
     * @param settings the local `Settings` to advertise — read from and written back through
     * during `send_handshake()`.
     * @param submiter callback that actually pushes the built handshake bytes out.
     */
    Handshake(Settings& settings, shared::SendCallback submiter) :
        m_local_settings{settings},
        m_submiter{std::move(submiter)}
    {
    }

    /**
     * @brief Server-side handshake step — sends the local SETTINGS frame (once, idempotent
     * across repeated calls) then checks whatever's arrived so far against the expected
     * connection preface.
     * @note Only enabled `requires(IsServer)` — the client overload right below covers the
     * other role. Same method name, mutually exclusive via the requires-clause, so only one
     * of the two ever actually exists for a given `IsServer` instantiation.
     * @param view the bytes received so far, checked against `HTTP2_CONNECTION_PREFACE`.
     * @param extension_registry the process's one `HttpExtensionRegistry`, forwarded to
     * `send_handshake()` to fold in any registered extension's local SETTINGS overrides.
     * @return `AWAITING_PREFACE` if not enough bytes yet, `COMPLETED` if the preface matched
     * (and got consumed off `view`), `PREFACE_ERROR` if what's there doesn't match.
     */
    HandshakeState
    process(utils::buffering::BufferReader& view, HttpExtensionRegistry& extension_registry)
        requires IsServer
    {
        core::logger::debug("http2/handshake", "process size={}", view.size());
        // Fire our own SETTINGS frame first (no-op after the first call thanks to the guard).
        send_handshake(extension_registry);

        // Then check whatever bytes have come in against the expected preface.
        return is_valid_preface(view);
    }

    /**
     * @brief Client-side handshake step — clients send the preface, they don't receive one, so
     * this just fires the SETTINGS frame (which also prepends the connection preface bytes,
     * see `send_handshake()`) and reports done immediately, no waiting-for-bytes state needed.
     * @note Only enabled `requires(!IsServer)` — mutually exclusive with the server overload
     * above via the requires-clause.
     * @param extension_registry the process's one `HttpExtensionRegistry`, forwarded to
     * `send_handshake()` to fold in any registered extension's local SETTINGS overrides.
     * @return always `COMPLETED` — there's nothing left to wait on from this side.
     */
    HandshakeState process(HttpExtensionRegistry& extension_registry)
        requires(!IsServer)
    {
        core::logger::debug("http2/handshake", "process");
        // Client's only job is to send its SETTINGS (with preface prepended) — no receive side.
        send_handshake(extension_registry);

        // Nothing to wait on from here — client handshake is done the moment bytes go out.
        return HandshakeState::COMPLETED;
    }

private:
    /**
     * @brief Checks `view`'s leading bytes against the fixed 24-byte HTTP/2 connection preface
     * (`"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"`), consuming them off `view` if they match.
     * @param view the bytes received so far.
     * @return `AWAITING_PREFACE` if `view` doesn't have 24 bytes yet, `COMPLETED` with the
     * preface consumed if it matches, `PREFACE_ERROR` if it's there but wrong.
     */
    HandshakeState is_valid_preface(utils::buffering::BufferReader& view) const
    {
        const auto& preface = HTTP2_CONNECTION_PREFACE;
        // Not enough bytes buffered yet to even compare — nothing to do but wait for more.
        if (view.size() < preface.size()) {
            core::logger::debug(
                "http2/handshake", "awaiting preface rx={} need={}", view.size(), preface.size()
            );

            return HandshakeState::AWAITING_PREFACE;
        }

        // Got enough bytes — compare the leading window against the fixed preface constant.
        if (std::ranges::equal(preface, view | std::views::take(preface.size()))) {
            core::logger::debug("http2/handshake", "valid preface");

            // Match — consume those bytes off the view so they don't get reprocessed as a
            // frame.
            view.consume(preface.size());
            return HandshakeState::COMPLETED;
        }

        // Bytes are there but don't match — this connection's cooked, straight PREFACE_ERROR.
        core::logger::warning("http2/handshake", "invalid preface");
        core::events::publish("http2.handshake.invalid_preface");
        return HandshakeState::PREFACE_ERROR;
    }

    /**
     * @brief Sends the initial local SETTINGS frame — client side also prepends the raw
     * connection preface bytes ahead of it in the same buffer node, server side sends just the
     * SETTINGS frame on its own. Guarded by `m_sent_settings` so repeated `process()` calls
     * (which happen naturally while waiting for more preface bytes server-side) don't
     * double-send.
     * @note This is the one real footgun in here: forget the `m_sent_settings` guard and every
     * partial-preface retry on the server side would blast out a duplicate SETTINGS frame. It's
     * guarded correctly as written, just flagging why the guard has to exist.
     */
    void send_handshake(HttpExtensionRegistry& extension_registry)
    {
        // Guard clause — already sent once, and server-side this can get called again on every
        // partial-preface retry, so bail here or we'd double-send SETTINGS.
        if (m_sent_settings) {
            core::logger::debug("http2/handshake", "settings already sent");

            return;
        }

        core::logger::debug("http2/handshake", "send settings");

        // Give every extension a chance to mutate the outgoing local settings (e.g. add RFC
        // 8441's SETTINGS_ENABLE_CONNECT_PROTOCOL via local.add_local_setting_override(...))
        // before they're serialized. No-op if no extensions are registered. Runs once per
        // connection thanks to the m_sent_settings guard above.
        extension_registry.for_each([&](auto& extension) {
            extension->on_local_settings(m_local_settings.get());
        });

        // Serialize the local settings into a SETTINGS frame payload first.
        auto payload = std::views::empty<std::byte> | WriteSettingsAdaptor{m_local_settings.get()} |
                       std::ranges::to<std::vector<std::byte>>();

        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::SETTINGS)
                         .add_flags(0)
                         .add_stream_id(0)
                         .add_payload(payload)
                         .build();

        // Client needs the connection preface bytes prepended ahead of the frame; server just
        // sends the SETTINGS frame on its own since the client already handled the preface.
        if constexpr (!IsServer) {
            auto size = HTTP2_CONNECTION_PREFACE.size() + frame.get_size();
            auto adaptor = WriteFrameBuilderAdaptor{
                std::move(frame), m_local_settings.get().get_max_frame_size()
            };
            auto node = std::span{HTTP2_CONNECTION_PREFACE} | adaptor |
                        std::ranges::to<utils::buffering::BufferNode>(size);
            m_submiter(std::move(node));
        } else {
            auto size = frame.get_size();
            auto adaptor = WriteFrameBuilderAdaptor{
                std::move(frame), m_local_settings.get().get_max_frame_size()
            };
            auto node = std::views::empty<std::byte> | adaptor |
                        std::ranges::to<utils::buffering::BufferNode>(size);
            m_submiter(std::move(node));
        }

        // Mark sent so any later calls this session short-circuit at the guard above.
        m_sent_settings = true;
    }

    std::reference_wrapper<Settings> m_local_settings;
    shared::SendCallback m_submiter;
    bool m_sent_settings{false};
};
} // namespace io::layer::http2

#ifdef CONGELADO_TEST
namespace io::layer::http2::handshake_tests {
using namespace boost::ut;

/// @brief Builds a BufferReader wrapping exactly `bytes` — same helper shape used across this
/// module's other partitions (see stream.cppm's make_reader).
static utils::buffering::BufferReader make_reader(const std::vector<std::byte>& bytes)
{
    auto* node = new utils::buffering::BufferNode(bytes.size());
    for (auto b: bytes) {
        node->push_back(b);
    }
    utils::buffering::BufferReader reader;
    reader.push_back(node);
    return reader;
}

suite<"Handshake<true> (server)"> server_handshake_suite = [] {
    "process() sends the local SETTINGS exactly once, even across repeated AWAITING_PREFACE calls"_test =
        [] {
            Settings local;
            int send_calls = 0;
            std::vector<std::byte> last_bytes;
            shared::SendCallback submiter = [&](utils::buffering::BufferNode&& node) {
                ++send_calls;
                last_bytes.assign(node.get_data(), node.get_data() + node.get_written());
            };
            Handshake<true> handshake{local, std::move(submiter)};
            HttpExtensionRegistry registry;

            std::vector<std::byte> partial(10, std::byte{0});
            auto reader = make_reader(partial);

            auto state1 = handshake.process(reader, registry);
            expect(state1 == HandshakeState::AWAITING_PREFACE);
            expect(send_calls == 1);

            auto state2 = handshake.process(reader, registry);
            expect(state2 == HandshakeState::AWAITING_PREFACE);
            expect(send_calls == 1); // guarded by m_sent_settings — no double-send

            auto header = last_bytes | ReadFrameHeaderAdaptor{local.get_max_frame_size()};
            expect(header.get_type() == shared_layer::FrameType::SETTINGS);
            expect(header.get_stream_id() == 0U);
        };

    "process() with a full valid preface returns COMPLETED and consumes exactly the preface bytes"_test =
        [] {
            Settings local;
            shared::SendCallback submiter = [](utils::buffering::BufferNode&& /*node*/) {};
            Handshake<true> handshake{local, std::move(submiter)};
            HttpExtensionRegistry registry;

            std::vector<std::byte> bytes(
                HTTP2_CONNECTION_PREFACE.begin(), HTTP2_CONNECTION_PREFACE.end()
            );
            bytes.push_back(std::byte{0xAA}); // trailing byte, should survive untouched
            auto reader = make_reader(bytes);

            auto state = handshake.process(reader, registry);
            expect(state == HandshakeState::COMPLETED);
            expect(reader.size() == 1U);
        };

    "process() with a mismatched preface returns PREFACE_ERROR"_test = [] {
        Settings local;
        shared::SendCallback submiter = [](utils::buffering::BufferNode&& /*node*/) {};
        Handshake<true> handshake{local, std::move(submiter)};
        HttpExtensionRegistry registry;

        std::vector<std::byte> bytes(24, std::byte{0x00});
        auto reader = make_reader(bytes);

        auto state = handshake.process(reader, registry);
        expect(state == HandshakeState::PREFACE_ERROR);
    };
};

suite<"Handshake<false> (client)"> client_handshake_suite = [] {
    "process() always returns COMPLETED and sends preface+SETTINGS exactly once"_test = [] {
        Settings local;
        int send_calls = 0;
        std::vector<std::byte> last_bytes;
        shared::SendCallback submiter = [&](utils::buffering::BufferNode&& node) {
            ++send_calls;
            last_bytes.assign(node.get_data(), node.get_data() + node.get_written());
        };
        Handshake<false> handshake{local, std::move(submiter)};
        HttpExtensionRegistry registry;

        auto state1 = handshake.process(registry);
        expect(state1 == HandshakeState::COMPLETED);
        expect(send_calls == 1);

        auto state2 = handshake.process(registry);
        expect(state2 == HandshakeState::COMPLETED);
        expect(send_calls == 1); // guarded by m_sent_settings — no double-send

        expect(last_bytes.size() >= HTTP2_CONNECTION_PREFACE.size()) << fatal;
        expect(
            std::ranges::equal(
                std::span(last_bytes).first(HTTP2_CONNECTION_PREFACE.size()),
                HTTP2_CONNECTION_PREFACE
            )
        );

        auto header = std::span(last_bytes).subspan(HTTP2_CONNECTION_PREFACE.size()) |
                      ReadFrameHeaderAdaptor{local.get_max_frame_size()};
        expect(header.get_type() == shared_layer::FrameType::SETTINGS);
        expect(header.get_stream_id() == 0U);
    };
};

} // namespace io::layer::http2::handshake_tests
#endif
