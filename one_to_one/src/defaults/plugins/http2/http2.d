module defaults.plugins.http2.http2;

// PORT-NOTE: This is a plugin .cc file — not @nogc at module level.
// The plugin binary is linked as a shared library and may use the GC.
// Only the on_load / on_unload hot paths annotate @nogc where possible.

import sdk.plugin.congelado_plugin;
import sdk.plugin.plugin;
import interfaces.interfaces;
import shared_.shared_;
import io.layer.http2.http2       : Http2Protocol;
import io.layer.http2.session     : Server;
import io.shared.http.types       : Protocol;
import io.shared.shared_          : Protocol;
import io.base.socket.socket      : Endpoint;
import io.base.flow.sync          : ServerFlowSocket;
import io.base.leverage.base      : Leverager, Context;
import io.base.socket.socket      : SocketProtocol;
import core_.config.types         : PluginConfig;
import core_.server.builder       : RouterContext, Route;
import core_.contract.contract    : ContractGroup;
import core_.logger.logger;
import util.optional              : Optional;
import util.alloc                 : make;

// ─── Http2Plugin ──────────────────────────────────────────────────────────────

private final class Http2Plugin : Plugin {
  public:
    override const(char)[] get_name()        const nothrow { return "http2";    }
    override const(char)[] get_version()     const nothrow { return "1.0.0";   }
    override const(char)[] get_unique_type() const nothrow { return "protocol"; }

    override const(const(char)[])[  ] get_requires() const nothrow {
        static immutable const(char)[][] reqs = ["FileLogger"];
        return reqs;
    }

    override uint capabilities() const nothrow { return CONGELADO_CAP_PROTOCOL; }

    override void on_load(HostCallbacks host, ConfigView cfg_view) {
        // Build PluginConfig from the flat key/value view.
        scope auto cfg = make!PluginConfig();
        cfg_view.for_each((const(char)[] key, const(char)[] val) @nogc nothrow {
            cfg.add_field(key, val);
        });

        // Construct Http2Protocol (owns TLS config, server builder, etc.).
        auto protocol = Http2Protocol(cfg_view.empty() ? null : cfg);

        m_server = protocol.get_server();

        // Wire HTTP route if the router context was provided.
        auto router_ctx = host.router_ctx!(
            RouterContext!(io.shared_.http.Protocol))();

        if (router_ctx !is null) {
            // PORT-NOTE: C++ lambda had noexcept; D delegate with @nogc nothrow.
            router_ctx.add_route(
                Route!(io.shared_.http.Protocol)("/hello").get(
                    (ref interfaces.IRequest!(io.shared_.http.Protocol),
                     ref interfaces.IResponse!(io.shared_.http.Protocol) res)
                            @nogc nothrow {
                        enum BODY = `{"message":"Hello from the HTTP/2 server plugin :D"}`;
                        // PORT-NOTE: C++ built std::vector<std::byte>; D uses stack buffer.
                        ubyte[128] body_buf;
                        size_t n = BODY.length < body_buf.length
                                        ? BODY.length : body_buf.length;
                        foreach (i, ch; cast(const(ubyte)[]) BODY[0 .. n])
                            body_buf[i] = ch;
                        res.set_status(interfaces.Status.OK);
                        res.add_header(io.shared_.http.Token.CONTENT_TYPE,
                                       "application/json");
                        res.set_body(body_buf[0 .. n]);
                    }));
        }

        auto contract_group =
            host.controller_ctx!(ContractGroup!())();
        auto leverager =
            host.leverager_ctx!(Leverager!Context)();

        if (contract_group is null || leverager is null) {
            core_.logger.error("http2", "no contract group or leverager");
            return;
        }

        core_.logger.important("http2", "listening on %s:%d",
                                protocol.get_bind_host().ptr,
                                protocol.get_bind_port());

        // Construct ServerFlowSocket for TLS connections.
        // PORT-NOTE: C++ used std::optional<ServerFlowSocket<...>>; D uses Optional.
        auto ep = Endpoint(protocol.get_bind_host(), protocol.get_bind_port());
        m_socket_flow = Optional!(ServerFlowSocket!(ContractGroup!(),
                                                    SocketProtocol.TLS))(
            ServerFlowSocket!(ContractGroup!(), SocketProtocol.TLS)(
                ep, *leverager, *contract_group));

        m_socket_flow.value.add_on_accept(
            (shared_.SendCallback send_cb,
             shared_.CloseCallback close_cb) @nogc nothrow
                    -> shared_.ReadCallback {
                return m_server.on_connect(send_cb, close_cb);
            });
        m_socket_flow.value.build();
    }

    override void on_unload() {
        // PORT-NOTE: C++ reset() freed the unique_ptr / optional in-place.
        // D: null out the references; underlying objects are GC-collected or disposed.
        m_socket_flow = Optional!(
            ServerFlowSocket!(ContractGroup!(), SocketProtocol.TLS)).init;
        m_server = null;
    }

    override void* protocol_get() nothrow {
        return cast(void*) m_server;
    }

  private:
    // PORT-NOTE: C++ used std::unique_ptr<Server> m_server.
    // D uses a plain class reference (GC managed in plugin binary).
    Server m_server = null;

    // PORT-NOTE: C++ used std::optional<ServerFlowSocket<...>>.
    // D uses Optional!ServerFlowSocket.
    Optional!(ServerFlowSocket!(ContractGroup!(), SocketProtocol.TLS)) m_socket_flow;
}

// CONGELADO_PLUGIN(Http2Plugin) — generates all extern(C) dlsym symbols.
mixin CongeladoPlugin!Http2Plugin;
