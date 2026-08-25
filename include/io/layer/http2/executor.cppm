export module io_layer_http2:executor;

import std;
import shared;
import core_logger;
import utils_buffering;
import io_layer_shared;
import :session;
import :frame;
import :extension;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::layer::http2 {

/**
 * @brief Per-session HTTP/2 frame executor: a `shared::HandlerBase` contract that owns frame
 * parsing, decoupling it from the socket read.
 *
 * The socket receiver hands newly-read bytes off via `feed()` and returns immediately (same spirit
 * as the http2 dispatch handing requests to `core::router::RouterExecutor`). This handler —
 * registered as its own `core::contract` worker and **infinitely self-rescheduling** like the
 * socket sender/receiver contracts — parses **one** frame off `m_handoff` per turn via
 * `Session::receive()`, then reschedules; buffered frames drain across successive turns.
 *
 * @warning Fixes the edge-triggered bug where a client that coalesces `HEADERS + DATA` into a
 * single write left the trailing frame stranded: `Session::receive()` parses one frame per call,
 * and the plain receiver only calls it when new bytes arrive — so a complete-but-unparsed frame
 * would sit forever with no further read to trigger it. This contract reschedules regardless of
 * reads, so the leftover is parsed on the next turn — no dependency on a future read.
 *
 * ## Why parsing is serial per connection
 * Frame demux is inherently sequential: you must read a frame's 9-byte header to learn its length
 * before the next frame boundary is known — no reading ahead. HPACK's dynamic table and the
 * connection-level flow-control window are ordered, shared state too. So one executor per
 * connection does all framing. Parallelism comes from (a) many connections, each with its own
 * executor, and (b) request handlers, which `core::router::RouterExecutor` runs on the worker pool
 * once a request has been demuxed and handed off.
 *
 * ## Buffer & guard
 * One buffer, `m_handoff`: `feed()` (socket receiver contract) splices bytes onto its tail;
 * `resume()` (this executor contract) parses off its head. `m_resume` guards `resume()` against
 * re-entrancy only — mirroring the sync Receiver's `m_stalled`; `feed()` is not guarded.
 */
class SessionExecutor : public ::shared::HandlerBase {
  public:
    /**
     * @brief Binds the executor to the session it drains frames into.
     * @param session the owning flow's `Session`; must outlive this executor (it's the flow's own
     * member, sitting right next to this one).
     */
    explicit SessionExecutor(Session &session) noexcept : m_session{session} {}

    /// @brief Contract-handler identity.
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "http2_session_executor";
    }

    /**
     * @brief The per-tick work: resume() and either reschedule or release depending on the outcome
     * — same release-on-done pattern as the sync Sender/Receiver.
     * @return the callable the contract worker invokes on schedule.
     */
    ::shared::WorkerFunction on_execute() override {
        return [this]() {
            if (resume()) {
                ::shared::this_handler::shedule();
            } else {
                ::shared::this_handler::release();
            }
        };
    }

    /**
     * @brief Parses one frame off `m_handoff` this turn, guarded against a concurrent drain by
     * `m_resume` (same re-entrancy pattern as the sync Sender/Receiver's resume()). Once a close
     * has been requested and everything's drained — buffer empty and the session has no active
     * streams — returns false so the caller releases this contract; otherwise returns true to keep
     * going.
     * @note One `receive()` per turn is enough: the contract reschedules unconditionally (see
     * on_execute()), so buffered-but-not-yet-parsed frames get picked up on the next turn without
     * depending on a socket read to trigger them.
     * @return false when closed and fully drained (release), true otherwise (keep rescheduling).
     */
    bool resume() {
        // Closed and nothing left to parse or serve — signal done so on_execute() releases.
        if (m_closing && m_handoff.empty() && m_session.get().is_idle()) {
            return false;
        }
        // Only parse if a drain isn't already running (guard).
        if (!get_resume()) {
            set_resume(true);
            auto &reader = m_handoff.get_view();
            if (!reader.empty()) {
                // WIREDUMP (temporary): dump what the parser is about to read off m_handoff.
                {
                    std::string hex;
                    std::size_t n = 0;
                    for (auto b : reader | std::views::take(48)) {
                        hex += std::format(
                            "{:02x} ", static_cast<unsigned>(std::to_integer<std::uint8_t>(b)));
                        ++n;
                    }
                    core::logger::warning("WIREDUMP", "engine-preparse size={} bytes[{}]: {}",
                                          reader.size(), n, hex);
                }
                m_session.get().receive(reader);
            }
            set_resume(false);
        }
        return true;
    }

    /**
     * @brief Hands newly-read frame bytes off to the executor. Called on the socket receiver
     * thread; moves the receiver's buffer nodes onto `m_handoff` (zero-copy splice) and returns
     * straight away — the actual parse runs later on this executor's worker turn.
     * @param view the receiver's buffer of freshly-read bytes; drained empty by the splice.
     */
    void feed(utils::buffering::BufferReader &view) { m_handoff.get_view().splice(view); }

    /**
     * @brief Requests a graceful stop. The executor keeps running (draining any bytes still in
     * flight) and only releases its own contract once everything's drained and the session has no
     * active streams left — so a final response/GOAWAY still gets parsed before this handler goes
     * idle. The owning flow polls the contract (see `is_idle()` there) to know when that happened.
     */
    void mark_close() noexcept { m_closing = true; }

  private:
    /// @brief Reads the guard (acquire ordering). Pairs with set_resume()'s release store — same
    /// approach as the sync Receiver/Sender's get_stalled()/set_stalled().
    [[nodiscard]] bool get_resume() const noexcept {
        return m_resume.load(std::memory_order_acquire);
    }
    /// @brief Sets the guard (release ordering).
    void set_resume(bool value) noexcept { m_resume.store(value, std::memory_order_release); }

    std::reference_wrapper<Session> m_session;
    utils::buffering::BufferWriter m_handoff;
    std::atomic<bool> m_resume{false};
    bool m_closing{false};
};

} // namespace io::layer::http2

#ifdef CONGELADO_TEST
namespace io::layer::http2::executor_tests {
using namespace boost::ut;

/// @brief Builds a BufferReader wrapping exactly `bytes` — same helper shape used across this
/// module's other partitions (see stream.cppm's make_reader).
static utils::buffering::BufferReader make_reader(const std::vector<std::byte> &bytes) {
    auto *node = new utils::buffering::BufferNode(bytes.size());
    for (auto b : bytes) {
        node->push_back(b);
    }
    utils::buffering::BufferReader reader;
    reader.push_back(node);
    return reader;
}

/// @brief Encodes a fully-built FrameBuilder into raw on-wire bytes (header + payload).
static std::vector<std::byte> encode_frame(FrameBuilder<shared_layer::FrameRole::SENDER> frame,
                                           std::uint32_t max_frame_size) {
    return WriteFrameBuilderAdaptor{std::move(frame), max_frame_size}() |
           std::ranges::to<std::vector<std::byte>>();
}

suite<"SessionExecutor"> session_executor_suite = [] {
    "ctor binds to the session; get_name() reports the fixed contract identity"_test = [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode &&) {}, [] {}, registry, Role::SERVER};
        SessionExecutor executor{session};

        expect(executor.get_name() == "http2_session_executor");
    };

    "resume() on an empty, non-closing executor is a no-op that keeps rescheduling (returns true)"_test =
        [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode &&) {}, [] {}, registry, Role::SERVER};
        SessionExecutor executor{session};

        expect(executor.resume());
    };

    "feed() hands bytes to the executor, and resume() parses a complete frame off them"_test = [] {
        HttpExtensionRegistry registry;
        int send_calls = 0;
        std::vector<std::byte> last_sent;
        Session session{
            [&](utils::buffering::BufferNode &&node) {
                ++send_calls;
                last_sent.assign(node.get_data(), node.get_data() + node.get_written());
            },
            [] {}, registry, Role::SERVER};
        SessionExecutor executor{session};

        // A non-ACK PING at the connection level — Session::receive() should reply with a PING
        // ACK carrying the same 8-byte payload, driven entirely through feed()+resume().
        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::PING)
                         .add_flags(0)
                         .add_stream_id(0)
                         .add_payload(std::vector<std::byte>(8, std::byte{0x11}))
                         .build();
        auto bytes =
            encode_frame(std::move(frame), session.get_local_settings().get_max_frame_size());
        auto reader = make_reader(bytes);

        executor.feed(reader);
        expect(reader.empty()); // splice() drains the source reader

        expect(executor.resume());
        expect(send_calls == 1) << fatal;

        auto reply_header =
            last_sent | ReadFrameHeaderAdaptor{session.get_local_settings().get_max_frame_size()};
        expect(reply_header.get_type() == shared_layer::FrameType::PING);
        expect((reply_header.get_flags() & shared_layer::Flags::ACK) != 0);
    };

    "mark_close() releases (resume() returns false) once drained and the session is idle"_test = [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode &&) {}, [] {}, registry, Role::SERVER};
        SessionExecutor executor{session};

        executor.mark_close();
        // Nothing was ever fed and the session never opened a stream — closing + empty + idle.
        expect(not executor.resume());
    };

    "mark_close() with bytes still queued keeps draining (returns true) until they're parsed"_test =
        [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode &&) {}, [] {}, registry, Role::SERVER};
        SessionExecutor executor{session};

        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::PING)
                         .add_flags(shared_layer::Flags::ACK)
                         .add_stream_id(0)
                         .add_payload(std::vector<std::byte>(8, std::byte{0x00}))
                         .build();
        auto bytes =
            encode_frame(std::move(frame), session.get_local_settings().get_max_frame_size());
        auto reader = make_reader(bytes);
        executor.feed(reader);

        executor.mark_close();
        // Still got a queued frame to drain — must keep rescheduling this turn.
        expect(executor.resume());
        // Drained now, closing, and the session never opened a stream — next resume() releases.
        expect(not executor.resume());
    };
};

} // namespace io::layer::http2::executor_tests
#endif
