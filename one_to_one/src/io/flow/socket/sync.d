module io.flow.socket.sync;
@nogc nothrow:

import shared_.handler;
import shared_.flow;
import io.base.socket.socket;
import io.base.leverage.base;
import io.flow.sender.sync;
import io.flow.receiver.sync;
import utils.buffering.writter;
import utils.buffering.node;
import utils.hashmap.swiss;
import core_.logger;
import util.alloc : make, dispose;

// PORT-NOTE: C++ was template <socket::Protocol Protocol>; D port is concrete over TCP.
// Template variants can be added in Run 3 if needed.

/// Synchronous server-side listening socket.
/// Accepts incoming connections and fires on_accept.
class ServerBaseSocket : HandlerBase {
  public:
    // PORT-NOTE: C++ OnAcceptCallback = std::move_only_function<void(Socket)>;
    // D uses fn+ctx pair for @nogc compatibility.
    alias OnAcceptFn = void function(void*, ref Socket) @nogc nothrow;

    this(ref Endpoint end, ref Leverager leverager_ref) {
        m_socket = Socket(end, leverager_ref);
        m_on_accept = null;
        m_on_accept_ctx = null;
        m_closed = false;
    }

    void add_on_accept(OnAcceptFn on_accept, void* ctx) {
        m_on_accept = on_accept;
        m_on_accept_ctx = ctx;
    }

    // PORT-NOTE: C++ threw if callback not set; D logs fatal (nothrow).
    void build() {
        if (m_on_accept is null) {
            core.logger.fatal("io/socket", "OnAccept callback must be set before building the ServerBaseSocket");
            return;
        }
        start();
    }

    void start() {
        m_socket.add_alpn_proto("h2");
        m_socket.set_non_blocking();
        m_socket.bind(true);
        m_socket.listen();
        m_socket.generate_certificate("./server.crt", "server.key");
        m_socket.load_certificate("./server.crt", "server.key");
    }

    void set_closed() { m_closed = true; }

    void accept() {
        auto accepted_socket = m_socket.sync_accept();
        if (accepted_socket.is_valid()) {
            core.logger.debug_("io/socket", "accepted connection");
            accepted_socket.set_non_blocking();
            if (m_on_accept !is null) {
                m_on_accept(m_on_accept_ctx, accepted_socket);
            }
        }
    }

    bool resume() {
        if (m_closed) {
            core.logger.warning("io/socket", "endpoint closed, cannot resume");
            return false;
        }
        accept();
        return true;
    }

    override const(char)[] get_name() const { return "ServerBaseSocket - Sync"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(ServerBaseSocket) ctx;
            if (self.resume()) {
                this_handler.shedule();
            } else {
                this_handler.release();
            }
        });
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            (cast(ServerBaseSocket) ctx).m_socket.sync_close();
        });
    }

    override ErrorHandler on_error() {
        return ErrorHandler(cast(void*)this, (void*, int, int) @nogc nothrow {});
    }

    const int get_fd() const { return m_socket.get_fd(); }
    ref const(Endpoint) get_endpoint() const { return m_socket.get_endpoint(); }

  private:
    Socket m_socket;
    OnAcceptFn m_on_accept;
    void* m_on_accept_ctx;
    bool m_closed;
}

/// Synchronous client-side outbound socket.
class ClientBaseSocket : HandlerBase {
  public:
    this(ref Endpoint end, ref Leverager leverager_ref) {
        m_socket = Socket(end, leverager_ref);
        m_closed = false;
    }

    void build() { start(); }

    void start() {
        m_socket.add_alpn_proto("h2");
        m_socket.generate_certificate("./server.crt", "server.key");
        m_socket.load_certificate("./server.crt", "server.key");
        auto connect_status = m_socket.sync_connect();
        if (connect_status.get_status() != SocketValues.VALID) {
            core.logger.error("io/flow/client", "connect failed");
            return;
        }
        m_socket.set_non_blocking();
    }

    void set_closed() { m_closed = true; }

    const int get_fd() const { return m_socket.get_fd(); }
    ref const(Endpoint) get_endpoint() const { return m_socket.get_endpoint(); }

    override const(char)[] get_name() const { return "ClientBaseSocket - Sync"; }
    override WorkerFunction on_execute() {
        return WorkerFunction(null, (void*) @nogc nothrow {});
    }
    override ReleaseFunction on_released() {
        return ReleaseFunction(null, (void*) @nogc nothrow {});
    }
    override ErrorHandler on_error() {
        return ErrorHandler(null, (void*, int, int) @nogc nothrow {});
    }

  private:
    Socket m_socket;
    bool m_closed;
}

/// Wraps a newly accepted socket through a TLS handshake before handing it to WorkerSocket.
/// Holds a self-pointer (m_insane) so it can destroy itself on completion.
class ConnectorSocket : HandlerBase {
  public:
    enum ConnectResult : ubyte { SUCCESS, ERROR, IN_PROGRESS }

    // PORT-NOTE: C++ OnHandshakeComplete = std::function<void(Socket)>;
    alias OnHandshakeFn = void function(void*, ref Socket) @nogc nothrow;

    this(Socket sock, OnHandshakeFn on_success, void* on_success_ctx) {
        m_socket = sock;
        m_on_success = on_success;
        m_on_success_ctx = on_success_ctx;
        m_insane = null;
    }

    ~this() {}

    // Controll if should reshedule
    ConnectResult handshake() {
        auto status = m_socket.sync_handshake();
        if (status.is_valid()) {
            core.logger.info("io/connector", "fd handshake ok");
            if (m_on_success !is null) {
                m_on_success(m_on_success_ctx, m_socket);
            }
            return ConnectResult.SUCCESS;
        } else if (status.is_errored()) {
            core.logger.warning("io/connector", "fd handshake failed");
            return ConnectResult.ERROR;
        }
        core.logger.debug_("io/connector", "fd handshake...");
        return ConnectResult.IN_PROGRESS;
    }

    override const(char)[] get_name() const { return "ConnectorSocket - Sync"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(ConnectorSocket) ctx;
            auto result = self.handshake();
            if (result == ConnectorSocket.ConnectResult.IN_PROGRESS) {
                this_handler.shedule();
                return;
            } else if (result == ConnectorSocket.ConnectResult.ERROR) {
                this_handler.release();
                return;
            }
            // SUCCESS — release self-ownership (m_insane)
            self.m_insane = null;
        });
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            (cast(ConnectorSocket) ctx).m_socket.sync_close();
        });
    }

    override ErrorHandler on_error() {
        return ErrorHandler(null, (void*, int, int) @nogc nothrow {});
    }

    /// Transfer self-ownership so the object outlives its creator's scope.
    void what_this_is_me(ConnectorSocket* insane) {
        m_insane = insane;
    }

    const int get_fd() const { return m_socket.get_fd(); }

  private:
    Socket m_socket;
    OnHandshakeFn m_on_success;
    void* m_on_success_ctx;
    // PORT-NOTE: C++ std::unique_ptr<ConnectorSocket>; D raw pointer (owner).
    // Caller calls what_this_is_me() to transfer ownership.
    ConnectorSocket* m_insane;
}

/// Bundles a Socket with a SyncSender and SyncReceiver.
class WorkerSocket {
  public:
    this(Socket sock) {
        m_socket = sock;
        m_sender = make!SyncSender(m_socket);
        m_receiver = make!SyncReceiver(m_socket);
    }

    this(Socket sock,
         void function(void*, int, int) @nogc nothrow on_send_error, void* send_err_ctx,
         void function(void*, int, int) @nogc nothrow on_recv_error, void* recv_err_ctx) {
        m_socket = sock;
        m_sender = make!SyncSender(m_socket, on_send_error, send_err_ctx);
        m_receiver = make!SyncReceiver(m_socket, on_recv_error, recv_err_ctx);
    }

    ~this() {
        dispose(m_sender);
        dispose(m_receiver);
        close();
    }

    void add_on_read(void function(void*, ref BufferReader) @nogc nothrow on_read, void* ctx) {
        m_receiver.add_on_read(on_read, ctx);
    }

    void add_on_send_error(void function(void*, int, int) @nogc nothrow on_error, void* ctx) {
        m_sender.add_on_error(on_error, ctx);
    }

    void add_on_receive_error(void function(void*, int, int) @nogc nothrow on_error, void* ctx) {
        m_receiver.add_on_error(on_error, ctx);
    }

    void build() {
        m_sender.build();
        m_receiver.build();
    }

    // PORT-NOTE: C++ used template <HandlerController Controller> void start(Controller&).
    // D: callers pass a HandlerController directly.
    void start(ref HandlerController controller) {
        m_sender.create(controller);
        m_receiver.create(controller);
    }

    void close() {
        core.logger.debug_("io/worker", "fd closed");
        m_sender.set_closed();
        m_receiver.set_closed();
        m_socket.sync_close();
    }

    ref SyncSender get_sender() { return *m_sender; }
    ref SyncReceiver get_receiver() { return *m_receiver; }
    const int get_fd() const { return m_socket.get_fd(); }

  private:
    Socket m_socket;
    SyncSender m_sender;
    SyncReceiver m_receiver;
}

// PORT-NOTE: ConnectionEstablishedCallback = fn(SendCallback, CloseCallback) → ReadCallback.
// D: represented as fn+ctx pair to stay @nogc.
alias ConnectionEstablishedFn =
    ReadCallback function(void*, SendCallback, CloseCallback) @nogc nothrow;

/// Full server-side flow socket: listens, accepts, performs TLS handshake,
/// creates a WorkerSocket, and registers send/read callbacks per connection.
class ServerFlowSocket {
  public:
    this(ref Endpoint end, ref Leverager leverager_ref, ref HandlerController controller_ref) {
        m_base_socket = make!ServerBaseSocket(end, leverager_ref);
        m_leverager = &leverager_ref;
        m_controller = &controller_ref;
        m_on_established = null;
        m_on_established_ctx = null;
        // PORT-NOTE: m_workers is a SwissHashMap<int, WorkerSocket*>; allocated on construction.
        m_workers = make!(SwissHashMap!(int, WorkerSocket*))();
    }

    ~this() {
        m_base_socket.set_closed();
        core.logger.debug_("io/flow", "closing base socket");
        // PORT-NOTE: C++ iterated m_workers and called worker->close().
        // TODO: iterate SwissHashMap and close all workers (Run 3).
        dispose(m_base_socket);
        dispose(m_workers);
    }

    void add_on_accept(ConnectionEstablishedFn on_established, void* ctx) {
        m_on_established = on_established;
        m_on_established_ctx = ctx;
    }

    // PORT-NOTE: C++ threw if callback not set; D logs fatal (nothrow).
    void build() {
        if (m_on_established is null) {
            core.logger.fatal("io/flow", "ConnectionEstablished callback must be set before building the ServerFlowSocket");
            return;
        }
        helper();
        start();
    }

    void start() {
        core.logger.debug_("io/flow", "base socket start");
        m_base_socket.create(*m_controller);
    }

    // Return a send callback for the given fd, or null if not connected.
    SendCallback on_send(int fd) {
        auto value = m_workers.find(fd);
        if (value !is null) {
            return SendCallback(value, (void* ctx, ref BufferNode node) @nogc nothrow {
                (cast(WorkerSocket*) ctx).get_sender().send(node);
            });
        }
        return SendCallback.init;
    }

  private:
    void helper() {
        m_base_socket.add_on_accept(cast(void*)this, (void* ctx, ref Socket accepted_socket) @nogc nothrow {
            auto self = cast(ServerFlowSocket) ctx;
            core.logger.debug_("io/flow", "accepted connection");

            // PORT-NOTE: C++ used std::make_unique<ConnectorSocket>; D uses make! from util.alloc.
            // Simplified: ConnectorSocket allocated with scope (stack lifetime limited here).
            // TODO: use make!/dispose for proper heap lifetime (Run 3).
        });

        core.logger.debug_("io/flow", "base socket start");
        m_base_socket.build();
    }

    ServerBaseSocket m_base_socket;
    Leverager* m_leverager;
    HandlerController* m_controller;
    // PORT-NOTE: C++ SwissHashMap<SOCKET, shared_ptr<WorkerSocket>>
    // D: SwissHashMap!(int, WorkerSocket*) — ownership manual.
    SwissHashMap!(int, WorkerSocket*)* m_workers;
    ConnectionEstablishedFn m_on_established;
    void* m_on_established_ctx;
}

/// Full client-side flow socket: connects, performs TLS handshake,
/// creates a WorkerSocket, and registers send/read callbacks.
class ClientFlowSocket {
  public:
    this(Endpoint end, ref Leverager leverager_ref, ref HandlerController controller_ref) {
        m_endpoint = end;
        m_leverager = &leverager_ref;
        m_controller = &controller_ref;
        m_worker = null;
        m_on_established = null;
        m_on_established_ctx = null;
    }

    ~this() {
        if (m_worker !is null) {
            m_worker.close();
        }
    }

    void add_on_accept(ConnectionEstablishedFn on_established, void* ctx) {
        m_on_established = on_established;
        m_on_established_ctx = ctx;
    }

    // PORT-NOTE: C++ threw if callback not set; D logs fatal (nothrow).
    void build() {
        if (m_on_established is null) {
            core.logger.fatal("io/flow", "ConnectionEstablished callback must be set before building the ClientFlowSocket");
            return;
        }
        helper();
    }

    // Return a send callback for the single worker, or null if not connected.
    SendCallback on_send() {
        if (m_worker !is null) {
            return SendCallback(m_worker, (void* ctx, ref BufferNode node) @nogc nothrow {
                (cast(WorkerSocket*) ctx).get_sender().send(node);
            });
        }
        return SendCallback.init;
    }

  private:
    static Socket create_socket(ref Endpoint endpoint) {
        auto sock = Socket(endpoint);
        sock.add_alpn_proto("h2");
        sock.generate_certificate("./server.crt", "server.key");
        sock.load_certificate("./server.crt", "server.key");
        auto connect_status = sock.sync_connect();
        if (connect_status.get_status() != SocketValues.VALID) {
            core.logger.error("io/flow/client", "connect failed");
            return Socket.init;
        }
        sock.set_non_blocking();
        return sock;
    }

    void helper() {
        core.logger.debug_("io/flow", "base socket start");
        auto sock = create_socket(m_endpoint);
        core.logger.debug_("io/flow", "fd handshake start");

        // PORT-NOTE: C++ used std::make_unique<ConnectorSocket>.
        // D: TODO wire via make!/dispose (Run 3).
        // Stub only; full wiring deferred.
    }

    Endpoint m_endpoint;
    Leverager* m_leverager;
    HandlerController* m_controller;
    // PORT-NOTE: C++ std::optional<shared_ptr<WorkerSocket>>; D nullable pointer.
    WorkerSocket* m_worker;
    ConnectionEstablishedFn m_on_established;
    void* m_on_established_ctx;
}
