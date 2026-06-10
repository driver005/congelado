module io.layer.http2.flow;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.flow.
// C++ ServerFlow and ClientFlow held Session + Handshake by value.
// D port: class members, same semantics.
// C++ used std::function for callbacks; D uses fn+ctx pairs (shared.flow).
// dynamic_cast<HttpRequest&> in on_submit → D is_cast / type check.

import io.layer.http2.consts;
import io.layer.http2.handshake;
import io.layer.http2.session;
import io.layer.http2.req : HttpRequest;
import io.error.http : Http2ErrorCode;
import utils.buffering.reader : BufferReader;
import utils.buffering.node   : BufferNode;
import shared.flow : ReadCallback, SendCallback, CloseCallback;
import interfaces.protocol : ReceiveDispatchFn, SendDispatchFn;

// ─── ServerFlow ──────────────────────────────────────────────────────────────

/// io::layer::http2::ServerFlow
class ServerFlow {
  public:
    this(SendCallback send, CloseCallback close,
         ReceiveDispatchFn dispatch = ReceiveDispatchFn.init) {
        m_session   = new Session(send, close, dispatch);
        m_handshake = new Handshake(
            m_session.get_local_settings(),
            SendCallback(cast(void*) m_session,
                (void* ctx, BufferNode node) @nogc nothrow {
                    (cast(Session) ctx).send_node(node);
                }),
            true /* is_server */);
        m_handshake_completed = false;
    }

    ReadCallback on_read() {
        return ReadCallback(cast(void*) this,
            (void* ctx, ref BufferReader view) @nogc nothrow {
                auto self = cast(ServerFlow) ctx;
                if (!self.m_handshake_completed) {
                    auto result = self.m_handshake.process_server(view);
                    if (result == HandshakeState.COMPLETED) {
                        self.m_handshake_completed = true;
                    } else if (result == HandshakeState.PREFACE_ERROR) {
                        self.m_session.close(Http2ErrorCode.PROTOCOL_ERROR);
                        return;
                    } else {
                        return;
                    }
                }
                if (view.size() > 0)
                    self.m_session.receive(view);
            });
    }

  private:
    Session   m_session;
    Handshake m_handshake;
    bool      m_handshake_completed;
}

// ─── ClientFlow ──────────────────────────────────────────────────────────────

/// io::layer::http2::ClientFlow
class ClientFlow {
  public:
    this(SendCallback send, CloseCallback close,
         ReceiveDispatchFn dispatch = ReceiveDispatchFn.init) {
        m_session   = new Session(send, close, dispatch);
        m_handshake = new Handshake(
            m_session.get_local_settings(),
            SendCallback(cast(void*) m_session,
                (void* ctx, BufferNode node) @nogc nothrow {
                    (cast(Session) ctx).send_node(node);
                }),
            false /* client */);
    }

    /// Returns a SendDispatchFn that submits requests on this session.
    SendDispatchFn on_submit() {
        return SendDispatchFn(cast(void*) m_session,
            (void* ctx, ref interfaces.request.IRequest!interfaces.protocol.HttpProtocol req) @nogc nothrow {
                auto session = cast(Session) ctx;
                // PORT-NOTE: C++ used dynamic_cast<HttpRequest&>.
                // D: explicit cast; if wrong type this is a silent no-op.
                auto http_req = cast(HttpRequest) req;
                if (http_req !is null)
                    session.send(http_req);
            });
    }

    /// Returns a callable that sends the handshake and returns the on_read callback.
    /// PORT-NOTE: C++ returned OnConnectCallback = std::function<ReadCallback()>.
    /// D: call on_connect() directly; it returns a ReadCallback.
    ReadCallback on_connect() {
        m_handshake.process_client();
        return ReadCallback(cast(void*) m_session,
            (void* ctx, ref BufferReader view) @nogc nothrow {
                if (view.size() > 0)
                    (cast(Session) ctx).receive(view);
            });
    }

  private:
    Session   m_session;
    Handshake m_handshake;
}
