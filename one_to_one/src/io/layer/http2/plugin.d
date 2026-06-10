module io.layer.http2.plugin;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.plugin.
// C++ plugin.cppm imported core_config, core_server (not yet ported).
// Those dependencies are forward-referenced as stubs here; Run 3 will wire them.
// Http2Protocol owns a server-side Server pool and an optional Client.
// std::unique_ptr → raw pointer ownership (caller calls dispose! in Run 3).

import io.layer.http2.flow;
import io.error.http : Http2ErrorCode;
import shared.flow   : ReadCallback, SendCallback, CloseCallback;
import interfaces.protocol : IProtocol, ReceiveDispatchFn, SendDispatchFn, HttpProtocol;
import util.alloc : make, dispose;

// PORT-NOTE: forward stubs for not-yet-ported core modules.
// Run 3: replace with proper imports once core/config and core/server are ported.
struct PluginConfig {
    // PORT-NOTE: C++ core::config::PluginConfig had get_fields() → map<string, string>.
    // D: placeholder struct; fields deferred to Run 3.
}

// ─── Client ──────────────────────────────────────────────────────────────────

/// io::layer::http2::Client
class Client {
  public:
    this(ReceiveDispatchFn dispatch) {
        m_flow     = null;
        m_dispatch = dispatch;
    }

    ~this() {
        dispose(m_flow);
        // PORT-NOTE: C++ default destructor.
    }

    @disable this(this);

    ReadCallback on_connect(SendCallback send, CloseCallback close) {
        m_flow = make!ClientFlow(send, close, m_dispatch);
        return m_flow.on_connect();
    }

    SendDispatchFn on_send() {
        if (m_flow is null) return SendDispatchFn.init;
        return m_flow.on_submit();
    }

  private:
    ClientFlow        m_flow;
    ReceiveDispatchFn m_dispatch;
}

// ─── Server ──────────────────────────────────────────────────────────────────

/// io::layer::http2::Server
class Server {
  public:
    this() {}

    ~this() {
        foreach (i; 0 .. m_flows_count)
            dispose(m_flows_buf[i]);
    }

    @disable this(this);

    // PORT-NOTE: C++ build(void*) cast to RouterContext<Protocol>*.
    // D: stub; core.server not yet ported.
    void build(void* router_ctx) {
        // TODO: wire core.server.RouteBuilder (Run 3).
    }

    ReadCallback on_connect(SendCallback send, CloseCallback close) {
        // Create a new ServerFlow for this connection.
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size ServerFlow[16] to avoid GC.
        assert(m_flows_count < m_flows_buf.length, "Server: too many flows");
        auto flow = make!ServerFlow(send, close, m_dispatch);
        m_flows_buf[m_flows_count++] = flow;
        return flow.on_read();
    }

    /// Slice accessor for active flows.
    ServerFlow[] m_flows() { return m_flows_buf[0 .. m_flows_count]; }

  private:
    // PORT-NOTE: C++ uses std::vector; D uses fixed-size ServerFlow[16] to avoid GC.
    ServerFlow[16]    m_flows_buf;
    size_t            m_flows_count;
    ReceiveDispatchFn m_dispatch;
    // PORT-NOTE: C++ std::optional<RouteHandler<Protocol>> m_server; deferred to Run 3.
}

// ─── Http2Protocol ───────────────────────────────────────────────────────────
// HTTP/2 protocol implementation.
// Handles per-connection ServerFlow creation and request dispatch.
// Transport binding (socket, thread pool) is owned by the plugin (http2.cc).

/// io::layer::http2::Http2Protocol
class Http2Protocol : IProtocol!(Server, Client) {
  public:
    // PORT-NOTE: C++ threw std::runtime_error on null config or missing fields.
    // D: silently uses defaults (nothrow).
    this(const(PluginConfig)* cfg = null) {
        m_host    = "localhost";
        m_port    = 8080;
        m_threads = 1;
        m_cert    = "server.crt";
        m_key     = "server.key";

        if (cfg !is null) {
            // TODO: parse host/port/cert/key/threads from cfg.get_fields() (Run 3).
        }
    }

    override const(char)[] get_protocol_name() const { return "http/2"; }
    override const(char)[] get_bind_host()     const { return m_host; }
    override ushort        get_bind_port()     const { return m_port; }
    override uint          get_bind_threads()  const { return m_threads; }
    override const(char)[] get_tls_cert()      const { return m_cert; }
    override const(char)[] get_tls_key()       const { return m_key; }

    // PORT-NOTE: C++ returned std::unique_ptr<Server/Client>; caller must dispose.
    override Server* get_server() { return make!Server(); }
    override Client* get_client(ReceiveDispatchFn dispatch) {
        return make!Client(dispatch);
    }

  private:
    const(char)[] m_host;
    ushort        m_port;
    uint          m_threads;
    const(char)[] m_cert;
    const(char)[] m_key;
}
