export module io_layer_http2:helper;

import std;
import io_layer_shared;
import :frame;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::layer::http2 {

class StreamStateMachine {
  public:
    /**
     * @brief Builds a state machine for the given stream, starting in IDLE per RFC 9113 §5.1 —
     * every stream begins idle before its first HEADERS/PUSH_PROMISE.
     * @param stream_id the stream id this machine tracks state for.
     */
    explicit StreamStateMachine(std::uint32_t stream_id) : m_id{stream_id} {}


    /**
     * @brief The full HTTP/2 stream state machine transition function (RFC 9113 §5.1) — feed it
     * the frame type/flags that just got sent or received and it either advances `m_state` or
     * throws if the transition is illegal for the current state.
     * @warning This is the single highest-stakes method in the whole HTTP/2 layer state-machine
     * wise. A few things to watch: PRIORITY and CONTINUATION never drive a transition at all
     * (early-return before the switch even runs) — RST_STREAM always slams straight to CLOSED
     * regardless of current state, no matter what state you're coming from. Miss either of
     * those special cases when reasoning about this and you'll assume states transition that
     * never actually do.
     * @warning HALF_CLOSED_LOCAL/HALF_CLOSED_REMOTE branch hard on `is_local` — sending
     * DATA/HEADERS while half-closed-local, or receiving them while half-closed-remote, is a
     * `StreamError` (not `ConnectionError`, that distinction matters for cleanup scope). Get
     * `is_local` backwards at a call site and this silently validates the wrong direction,
     * that's a real L waiting to happen for whoever wires this up wrong.
     * @param TYPE the frame type driving this transition.
     * @param FLAGS the frame's raw flags byte — only END_STREAM is inspected here.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state after applying the transition (may be unchanged from
     * before, e.g. staying OPEN).
     * @throws error::http::ConnectionError for protocol-violating transitions (illegal frame on
     * IDLE/RESERVED_LOCAL/RESERVED_REMOTE/CLOSED).
     * @throws error::http::StreamError for DATA/HEADERS sent or received against the wrong
     * direction on a half-closed stream.
     */
    shared_layer::StreamState advance(const shared_layer::FrameType &TYPE, const std::uint8_t &FLAGS, bool is_local) {
        const bool END_STREAM = (FLAGS & shared_layer::Flags::END_STREAM) != 0;

        // PRIORITY and CONTINUATION do not drive state transitions (§5.1).
        // RST_STREAM always moves to Closed regardless of current state.
        if (TYPE == shared_layer::FrameType::PRIORITY) {
            return m_state;
        }

        if (TYPE == shared_layer::FrameType::RST_STREAM) {
            require_not_idle(TYPE);
            return apply(shared_layer::StreamState::CLOSED);
        }

        // Everything past here is the per-current-state transition table — RFC 9113 §5.1's
        // diagram translated straight into a switch, one handler method per state below.
        switch (m_state) {
        case shared_layer::StreamState::IDLE:
            return handle_idle(TYPE, END_STREAM, is_local);
        case shared_layer::StreamState::OPEN:
            return handle_open(END_STREAM, is_local);
        case shared_layer::StreamState::HALF_CLOSED_LOCAL:
            return handle_half_closed_local(TYPE, END_STREAM, is_local);
        case shared_layer::StreamState::HALF_CLOSED_REMOTE:
            return handle_half_closed_remote(TYPE, END_STREAM, is_local);
        case shared_layer::StreamState::RESERVED_LOCAL:
            return handle_reserved_local(TYPE, is_local);
        case shared_layer::StreamState::RESERVED_REMOTE:
            return handle_reserved_remote(TYPE, is_local);
        case shared_layer::StreamState::CLOSED:
            return handle_closed(TYPE, is_local);
        }
    }

    /**
     * @brief Grabs the current stream state.
     * @return the stream's current state.
     */
    [[nodiscard]] const shared_layer::StreamState &get_state() const noexcept { return m_state; }
    /**
     * @brief Grabs the stream id this machine tracks.
     * @return the stream id.
     */
    [[nodiscard]] const std::uint32_t &id() const noexcept { return m_id; }

    /**
     * @brief Checks whether the stream is fully OPEN (neither side has half-closed). Bet, plain
     * state check.
     * @return true if the state is exactly OPEN.
     */
    [[nodiscard]] bool is_open() const noexcept { return m_state == shared_layer::StreamState::OPEN; }
    /**
     * @brief Checks whether it's still legal for us to send DATA on this stream.
     * @return true if OPEN or HALF_CLOSED_REMOTE — the two states where our send direction is
     * still alive.
     */
    [[nodiscard]] bool can_send_data() const noexcept {
        return m_state == shared_layer::StreamState::OPEN || m_state == shared_layer::StreamState::HALF_CLOSED_REMOTE;
    }
    /**
     * @brief Checks whether it's still legal to receive DATA on this stream.
     * @return true if OPEN or HALF_CLOSED_LOCAL — the two states where the peer's send
     * direction is still alive.
     */
    [[nodiscard]] bool can_receive_data() const noexcept {
        return m_state == shared_layer::StreamState::OPEN || m_state == shared_layer::StreamState::HALF_CLOSED_LOCAL;
    }
    /**
     * @brief Checks whether the stream has fully closed. Lowkey the terminal state check.
     * @return true if the state is CLOSED.
     */
    [[nodiscard]] bool is_closed() const noexcept { return m_state == shared_layer::StreamState::CLOSED; }

  private:
    /**
     * @brief Sets `m_state` to `next` and hands it straight back — the one spot every actual
     * state mutation in advance() funnels through, keeps the write and the return value
     * consistent in one place instead of duplicating `m_state = next; return m_state;` at every
     * call site.
     * @param next the state to transition into.
     * @return `next`, echoed back after being stored.
     */
    shared_layer::StreamState apply(shared_layer::StreamState next) noexcept {
        m_state = next;
        return m_state;
    }

    /**
     * @brief advance()'s IDLE-state handler (RFC 9113 §5.1) — HEADERS opens the stream (straight
     * to half-closed if END_STREAM rides along), PUSH_PROMISE reserves it, anything else is a
     * protocol error.
     * @param TYPE the frame type driving this transition.
     * @param end_stream whether the frame carried END_STREAM.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state.
     * @throws error::http::ConnectionError for anything other than HEADERS/PUSH_PROMISE.
     */
    shared_layer::StreamState handle_idle(const shared_layer::FrameType &TYPE, bool end_stream,
                                          bool is_local) {
        if (TYPE == shared_layer::FrameType::HEADERS) {
            // Sending or receiving HEADERS opens the stream.
            // If END_STREAM is also set, jump straight to half-closed.
            if (end_stream) {
                return apply(is_local ? shared_layer::StreamState::HALF_CLOSED_LOCAL
                                      : shared_layer::StreamState::HALF_CLOSED_REMOTE);
            }
            return apply(shared_layer::StreamState::OPEN);
        }
        if (TYPE == shared_layer::FrameType::PUSH_PROMISE) {
            // PUSH_PROMISE transitions the *promised* (this) stream.
            // Sending → ReservedLocal; receiving → ReservedRemote.
            return apply(is_local ? shared_layer::StreamState::RESERVED_LOCAL
                                  : shared_layer::StreamState::RESERVED_REMOTE);
        }
        // Anything else on an idle stream is a PROTOCOL_ERROR (§5.1).
        throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                           std::format("FrameBuilder type `{}` received on idle stream", TYPE),
                                           m_id);
    }

    /**
     * @brief advance()'s OPEN-state handler — END_STREAM half-closes it (direction depends on
     * who sent it), otherwise the stream stays fully open.
     * @param end_stream whether the frame carried END_STREAM.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state.
     */
    shared_layer::StreamState handle_open(bool end_stream, bool is_local) {
        // END_STREAM on an open stream half-closes it — direction depends on who sent it.
        if (end_stream) {
            return apply(is_local ? shared_layer::StreamState::HALF_CLOSED_LOCAL
                                  : shared_layer::StreamState::HALF_CLOSED_REMOTE);
        }
        // No END_STREAM — nothing to do, still fully open.
        return m_state; // stays Open
    }

    /**
     * @brief advance()'s HALF_CLOSED_LOCAL-state handler — sending DATA/HEADERS from here is a
     * StreamError; receiving END_STREAM closes the stream.
     * @param TYPE the frame type driving this transition.
     * @param end_stream whether the frame carried END_STREAM.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state.
     * @throws error::http::StreamError if we try to send DATA/HEADERS while half-closed-local.
     */
    shared_layer::StreamState handle_half_closed_local(const shared_layer::FrameType &TYPE,
                                                       bool end_stream, bool is_local) {
        if (is_local) {
            // We must not send data or headers frames on a half-closed-local
            // stream (WINDOW_UPDATE and RST_STREAM are handled above / below).
            if (TYPE == shared_layer::FrameType::DATA || TYPE == shared_layer::FrameType::HEADERS) {
                throw error::http::StreamError(m_id, error::http::Http2ErrorCode::STREAM_CLOSED,
                                               "Cannot send DATA/HEADERS on half-closed (local) stream");
            }
        } else {
            // Receiving END_STREAM closes the stream.
            if (end_stream) {
                return apply(shared_layer::StreamState::CLOSED);
            }
        }
        return m_state;
    }

    /**
     * @brief advance()'s HALF_CLOSED_REMOTE-state handler — receiving DATA/HEADERS here is a
     * StreamError; sending END_STREAM closes the stream.
     * @param TYPE the frame type driving this transition.
     * @param end_stream whether the frame carried END_STREAM.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state.
     * @throws error::http::StreamError if the peer sends DATA/HEADERS while half-closed-remote.
     */
    shared_layer::StreamState handle_half_closed_remote(const shared_layer::FrameType &TYPE,
                                                        bool end_stream, bool is_local) {
        if (!is_local) {
            // Peer must not send DATA or HEADERS on a half-closed-remote stream.
            if (TYPE == shared_layer::FrameType::DATA || TYPE == shared_layer::FrameType::HEADERS) {
                throw error::http::StreamError(m_id, error::http::Http2ErrorCode::STREAM_CLOSED,
                                               "Received DATA/HEADERS on half-closed (remote) stream");
            }
        } else {
            // Sending END_STREAM closes the stream.
            if (end_stream) {
                return apply(shared_layer::StreamState::CLOSED);
            }
        }
        return m_state;
    }

    /**
     * @brief advance()'s RESERVED_LOCAL-state handler — our follow-up HEADERS moves it to
     * half-closed-remote; the peer may still adjust flow-control or cancel via WINDOW_UPDATE/
     * RST_STREAM; anything else is off-script.
     * @param TYPE the frame type driving this transition.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state.
     * @throws error::http::ConnectionError for anything not covered above.
     */
    shared_layer::StreamState handle_reserved_local(const shared_layer::FrameType &TYPE, bool is_local) {
        // We pushed this promise, so we're the one who follows up with the actual HEADERS —
        // that moves it on to half-closed-remote (peer still owes its response).
        if (is_local && TYPE == shared_layer::FrameType::HEADERS) {
            return apply(shared_layer::StreamState::HALF_CLOSED_REMOTE);
        }
        // Peer's still lowkey allowed to adjust flow-control or bail on the pushed stream.
        if (!is_local &&
            (TYPE == shared_layer::FrameType::WINDOW_UPDATE || TYPE == shared_layer::FrameType::RST_STREAM)) {
            return m_state; // peer may send these
        }
        // Anything else here is off-script for a reserved-local stream.
        throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                           std::format("Illegal frame type `{}` on reserved (local) stream", TYPE),
                                           m_id);
    }

    /**
     * @brief advance()'s RESERVED_REMOTE-state handler — the peer's follow-up HEADERS moves it
     * to half-closed-local; we may still adjust flow-control or cancel via WINDOW_UPDATE/
     * RST_STREAM; anything else is off-script.
     * @param TYPE the frame type driving this transition.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's new state.
     * @throws error::http::ConnectionError for anything not covered above.
     */
    shared_layer::StreamState handle_reserved_remote(const shared_layer::FrameType &TYPE, bool is_local) {
        // Peer pushed this promise and now follows up with HEADERS — moves us to
        // half-closed-local (we still owe the response on our side).
        if (!is_local && TYPE == shared_layer::FrameType::HEADERS) {
            return apply(shared_layer::StreamState::HALF_CLOSED_LOCAL);
        }
        // We're still allowed to adjust flow-control or cancel the pushed stream ourselves.
        if (is_local &&
            (TYPE == shared_layer::FrameType::WINDOW_UPDATE || TYPE == shared_layer::FrameType::RST_STREAM)) {
            return m_state;
        }

        // Anything else here is off-script for a reserved-remote stream.
        throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                           std::format("Illegal frame type `{}` on reserved (remote) stream", TYPE),
                                           m_id);
    }

    /**
     * @brief advance()'s CLOSED-state handler — WINDOW_UPDATE/RST_STREAM from the peer are
     * tolerated (post-close race per §5.1); DATA/HEADERS from the peer or anything else is a
     * connection error.
     * @param TYPE the frame type driving this transition.
     * @param is_local true if this frame is being sent by us, false if received from the peer.
     * @return the stream's (unchanged) state.
     * @throws error::http::ConnectionError for DATA/HEADERS or anything not covered above.
     */
    shared_layer::StreamState handle_closed(const shared_layer::FrameType &TYPE, bool is_local) {
        // WINDOW_UPDATE and RST_STREAM may arrive briefly after closure
        // due to race conditions — permit them silently (§5.1).
        if (!is_local &&
            (TYPE == shared_layer::FrameType::WINDOW_UPDATE || TYPE == shared_layer::FrameType::RST_STREAM)) {
            return m_state;
        }

        // DATA or HEADERS on a closed stream → connection error
        if (!is_local && (TYPE == shared_layer::FrameType::DATA || TYPE == shared_layer::FrameType::HEADERS)) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::STREAM_CLOSED,
                                               std::format("Received `{}` on closed stream", TYPE), m_id);
        }
        // Anything else: connection error per §5.1
        throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                           std::format("FrameBuilder type `{}` received on closed stream", TYPE),
                                           m_id);
    }

    /**
     * @brief Guard used by RST_STREAM handling in advance() — a stream still sitting in IDLE
     * has never been opened, so an RST_STREAM against it is a protocol violation, not a normal
     * close.
     * @param type the frame type to name in the thrown error (always RST_STREAM in practice,
     * this is the only call site).
     * @throws error::http::ConnectionError if the stream is currently IDLE.
     */
    void require_not_idle(shared_layer::FrameType type) const {
        // Guard clause — an RST_STREAM against a never-opened stream is a protocol violation.
        if (m_state == shared_layer::StreamState::IDLE) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               std::format("FrameBuilder type `{}` received on idle stream", type),
                                               m_id);
        }
    }

    std::uint32_t m_id;
    shared_layer::StreamState m_state{shared_layer::StreamState::IDLE};
};

} // namespace io::layer::http2

#ifdef CONGELADO_TEST
namespace io::layer::http2::tests {
using namespace boost::ut;
using shared_layer::FrameType;
using shared_layer::Flags;
using shared_layer::StreamState;

suite<"StreamStateMachine basics"> stream_state_machine_basics_suite = [] {
    "starts IDLE and remembers its stream id"_test = [] {
        StreamStateMachine machine{5};

        expect(machine.id() == 5U);
        expect(machine.get_state() == StreamState::IDLE);
        expect(not machine.is_open());
        expect(not machine.is_closed());
    };
};

suite<"StreamStateMachine IDLE transitions"> stream_state_machine_idle_suite = [] {
    "HEADERS without END_STREAM opens the stream"_test = [] {
        StreamStateMachine machine{1};
        auto state = machine.advance(FrameType::HEADERS, 0, false);

        expect(state == StreamState::OPEN);
        expect(machine.is_open());
    };

    "HEADERS with END_STREAM half-closes local or remote depending on who sent it"_test = [] {
        StreamStateMachine local_sender{1};
        auto local_state = local_sender.advance(FrameType::HEADERS, Flags::END_STREAM, true);
        expect(local_state == StreamState::HALF_CLOSED_LOCAL);

        StreamStateMachine remote_sender{3};
        auto remote_state = remote_sender.advance(FrameType::HEADERS, Flags::END_STREAM, false);
        expect(remote_state == StreamState::HALF_CLOSED_REMOTE);
    };

    "PUSH_PROMISE reserves the stream, direction depending on who sent it"_test = [] {
        StreamStateMachine local_push{2};
        expect(local_push.advance(FrameType::PUSH_PROMISE, 0, true) == StreamState::RESERVED_LOCAL);

        StreamStateMachine remote_push{4};
        expect(remote_push.advance(FrameType::PUSH_PROMISE, 0, false) ==
               StreamState::RESERVED_REMOTE);
    };

    "anything else on an idle stream is a protocol error"_test = [] {
        StreamStateMachine machine{1};
        expect(throws<error::http::ConnectionError>(
            [&] { std::ignore = machine.advance(FrameType::DATA, 0, false); }));
    };
};

suite<"StreamStateMachine OPEN transitions"> stream_state_machine_open_suite = [] {
    "stays open without END_STREAM"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, 0, false);

        auto state = machine.advance(FrameType::DATA, 0, false);
        expect(state == StreamState::OPEN);
    };

    "END_STREAM half-closes local when we sent it, remote when the peer did"_test = [] {
        StreamStateMachine sender{1};
        sender.advance(FrameType::HEADERS, 0, false);
        expect(sender.advance(FrameType::DATA, Flags::END_STREAM, true) ==
               StreamState::HALF_CLOSED_LOCAL);

        StreamStateMachine receiver{3};
        receiver.advance(FrameType::HEADERS, 0, false);
        expect(receiver.advance(FrameType::DATA, Flags::END_STREAM, false) ==
               StreamState::HALF_CLOSED_REMOTE);
    };
};

suite<"StreamStateMachine HALF_CLOSED_LOCAL transitions"> stream_state_machine_hcl_suite = [] {
    "sending DATA/HEADERS while half-closed-local is a stream error"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, true);
        expect(machine.get_state() == StreamState::HALF_CLOSED_LOCAL);

        expect(throws<error::http::StreamError>(
            [&] { std::ignore = machine.advance(FrameType::DATA, 0, true); }));
    };

    "receiving END_STREAM closes the stream"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, true);

        auto state = machine.advance(FrameType::DATA, Flags::END_STREAM, false);
        expect(state == StreamState::CLOSED);
        expect(machine.is_closed());
    };

    "receiving without END_STREAM stays half-closed-local"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, true);

        auto state = machine.advance(FrameType::WINDOW_UPDATE, 0, false);
        expect(state == StreamState::HALF_CLOSED_LOCAL);
    };
};

suite<"StreamStateMachine HALF_CLOSED_REMOTE transitions"> stream_state_machine_hcr_suite = [] {
    "receiving DATA/HEADERS while half-closed-remote is a stream error"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, false);
        expect(machine.get_state() == StreamState::HALF_CLOSED_REMOTE);

        expect(throws<error::http::StreamError>(
            [&] { std::ignore = machine.advance(FrameType::HEADERS, 0, false); }));
    };

    "sending END_STREAM closes the stream"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, false);

        auto state = machine.advance(FrameType::DATA, Flags::END_STREAM, true);
        expect(state == StreamState::CLOSED);
    };
};

suite<"StreamStateMachine RESERVED transitions"> stream_state_machine_reserved_suite = [] {
    "reserved-local: our follow-up HEADERS moves to half-closed-remote"_test = [] {
        StreamStateMachine machine{2};
        machine.advance(FrameType::PUSH_PROMISE, 0, true);

        auto state = machine.advance(FrameType::HEADERS, 0, true);
        expect(state == StreamState::HALF_CLOSED_REMOTE);
    };

    "reserved-local: peer WINDOW_UPDATE/RST_STREAM tolerated, anything else errors"_test = [] {
        StreamStateMachine machine{2};
        machine.advance(FrameType::PUSH_PROMISE, 0, true);

        auto state = machine.advance(FrameType::WINDOW_UPDATE, 0, false);
        expect(state == StreamState::RESERVED_LOCAL);

        StreamStateMachine other{4};
        other.advance(FrameType::PUSH_PROMISE, 0, true);
        expect(throws<error::http::ConnectionError>(
            [&] { std::ignore = other.advance(FrameType::DATA, 0, false); }));
    };

    "reserved-remote: peer's follow-up HEADERS moves to half-closed-local"_test = [] {
        StreamStateMachine machine{2};
        machine.advance(FrameType::PUSH_PROMISE, 0, false);

        auto state = machine.advance(FrameType::HEADERS, 0, false);
        expect(state == StreamState::HALF_CLOSED_LOCAL);
    };

    "reserved-remote: our WINDOW_UPDATE/RST_STREAM tolerated, anything else errors"_test = [] {
        StreamStateMachine machine{2};
        machine.advance(FrameType::PUSH_PROMISE, 0, false);

        auto state = machine.advance(FrameType::WINDOW_UPDATE, 0, true);
        expect(state == StreamState::RESERVED_REMOTE);

        StreamStateMachine other{4};
        other.advance(FrameType::PUSH_PROMISE, 0, false);
        expect(throws<error::http::ConnectionError>(
            [&] { std::ignore = other.advance(FrameType::DATA, 0, true); }));
    };
};

suite<"StreamStateMachine CLOSED transitions"> stream_state_machine_closed_suite = [] {
    "post-close WINDOW_UPDATE/RST_STREAM from the peer are tolerated"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, true);
        machine.advance(FrameType::DATA, Flags::END_STREAM, false);
        expect(machine.is_closed());

        auto state = machine.advance(FrameType::WINDOW_UPDATE, 0, false);
        expect(state == StreamState::CLOSED);
    };

    "DATA/HEADERS from the peer on a closed stream is a connection error"_test = [] {
        StreamStateMachine machine{1};
        machine.advance(FrameType::HEADERS, Flags::END_STREAM, true);
        machine.advance(FrameType::DATA, Flags::END_STREAM, false);

        expect(throws<error::http::ConnectionError>(
            [&] { std::ignore = machine.advance(FrameType::DATA, 0, false); }));
    };
};

suite<"StreamStateMachine PRIORITY / RST_STREAM special-casing"> stream_state_machine_special_suite =
    [] {
        "PRIORITY never drives a transition, in any state"_test = [] {
            StreamStateMachine machine{1};
            expect(machine.advance(FrameType::PRIORITY, 0, false) == StreamState::IDLE);

            machine.advance(FrameType::HEADERS, 0, false);
            expect(machine.advance(FrameType::PRIORITY, 0, false) == StreamState::OPEN);
        };

        "RST_STREAM always slams straight to CLOSED once the stream isn't idle"_test = [] {
            StreamStateMachine machine{1};
            machine.advance(FrameType::HEADERS, 0, false);

            auto state = machine.advance(FrameType::RST_STREAM, 0, false);
            expect(state == StreamState::CLOSED);
        };

        "RST_STREAM against a never-opened (idle) stream is a protocol violation"_test = [] {
            StreamStateMachine machine{1};
            expect(throws<error::http::ConnectionError>(
                [&] { std::ignore = machine.advance(FrameType::RST_STREAM, 0, false); }));
        };
    };

suite<"StreamStateMachine capability checks"> stream_state_machine_capability_suite = [] {
    "can_send_data/can_receive_data reflect OPEN and the matching half-closed state"_test = [] {
        StreamStateMachine open_machine{1};
        open_machine.advance(FrameType::HEADERS, 0, false);
        expect(open_machine.can_send_data());
        expect(open_machine.can_receive_data());

        StreamStateMachine half_closed_local{3};
        half_closed_local.advance(FrameType::HEADERS, Flags::END_STREAM, true);
        expect(not half_closed_local.can_send_data());
        expect(half_closed_local.can_receive_data());

        StreamStateMachine half_closed_remote{5};
        half_closed_remote.advance(FrameType::HEADERS, Flags::END_STREAM, false);
        expect(half_closed_remote.can_send_data());
        expect(not half_closed_remote.can_receive_data());
    };
};

} // namespace io::layer::http2::tests
#endif
