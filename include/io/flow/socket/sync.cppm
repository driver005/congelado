export module io_flow_socket:sync;

import std;
import core_logger;
import io_base_socket;
import utils_buffering;
import io_base_leverage;
import io_flow_sender;
import io_flow_receiver;
import interfaces;
import shared;
import hashmap;

export namespace io::base::flow::sync {

using Leverager = leverage::Leverager<leverage::Context>;

template <socket::Protocol Protocol>
class ServerBaseSocket : public shared::HandlerBase {
  public:
    using OnAcceptCallback = std::move_only_function<void(socket::Socket<Protocol>)>;

    /**
     * @brief Builds a server-side base socket bound to `leverager`'s io context — doesn't
     * actually bind/listen yet, that's on start()/build().
     * @param end the endpoint to eventually bind and listen on.
     * @param leverager the io_uring leverager backing this socket.
     */
    ServerBaseSocket(socket::Endpoint end, Leverager &leverager)
        : m_socket{socket::Socket<Protocol, true>{std::move(end), std::ref(leverager)}} {};

    /**
     * @brief Default dtor — no special teardown, on_released() handles socket close.
     */
    ~ServerBaseSocket() override = default;

    /**
     * @brief Copy ctor deleted — sockets aren't copyable, straight up.
     */
    ServerBaseSocket(const ServerBaseSocket &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    ServerBaseSocket &operator=(const ServerBaseSocket &) = delete;
    /**
     * @brief Move ctor — steals the underlying socket, accept callback, and closed flag off
     * `other`, leaving it in a moved-from state.
     * @param other the socket to move from.
     */
    ServerBaseSocket(ServerBaseSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_on_accept{std::move(other.m_on_accept)},
          m_closed{other.m_closed} {}
    /**
     * @brief Move assignment — self-assignment guarded, steals `other`'s state.
     * @param other the socket to move from.
     * @return `*this`.
     */
    ServerBaseSocket &operator=(ServerBaseSocket &&other) noexcept {
        // Guard self-assignment, then steal every bit of `other`'s state.
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_on_accept = std::move(other.m_on_accept);
            m_closed = other.m_closed;
        }
        return *this;
    };

    /**
     * @brief Wires the callback invoked every time a new connection gets accepted.
     * @param on_accept the accept callback.
     */
    void add_on_accept(OnAcceptCallback on_accept) & { m_on_accept = std::move(on_accept); }

    /**
     * @brief Validates the accept callback is set, then kicks off start() — bind, listen, TLS
     * cert generation/load, the whole startup sequence.
     * @throws std::runtime_error if the accept callback hasn't been set yet.
     */
    void build() & {
        // Accept callback's mandatory before this socket can do anything useful.
        if (!m_on_accept) {
            throw std::runtime_error(
                "OnAccept callback must be set before building the ServerBaseSocket");
        }
        // Callback confirmed — go through the full bind/listen/TLS bring-up sequence.
        start();
    }

    /**
     * @brief Runs the actual server bring-up sequence: registers the h2 ALPN proto, flips
     * non-blocking, binds, starts listening, then generates and loads a TLS cert/key pair.
     * @warning Cert/key paths are hardcoded (`"./server.crt"`, `"server.key"`) — no way to
     * configure these from outside right now.
     */
    void start() {
        // Advertise h2 over ALPN, then flip non-blocking before touching the socket further.
        m_socket.add_alpn_proto("h2");
        m_socket.set_non_blocking();
        // Bind + listen to actually start accepting connections.
        m_socket.bind(true);
        m_socket.listen();
        // Generate then immediately load the TLS cert/key pair the handshake will need.
        m_socket.generate_certificate("./server.crt", "server.key");
        m_socket.load_certificate("./server.crt", "server.key");
    }

    /**
     * @brief Marks this socket closed — resume() becomes a permanent no-op after this.
     */
    void set_closed() { m_closed = true; }

    /**
     * @brief Blocks on a sync accept and, if a connection actually landed, flips it non-blocking
     * and forwards it to `m_on_accept`.
     * @note A "no connection" accept result is silently swallowed — `accepted_socket` just has
     * to be truthy for anything to happen, no error gets surfaced for the empty case.
     */
    void accept() {
        // Blocks waiting for a connection — an empty/falsy result just means nothing landed.
        socket::Socket<Protocol> accepted_socket = m_socket.sync_accept();
        if (accepted_socket) {
            // Got one — flip it non-blocking and hand it off to the accept callback.
            core::logger::debug("io/socket", "accepted {} from {}",
                                m_socket.get_endpoint().to_string(),
                                accepted_socket.get_endpoint().to_string());
            accepted_socket.set_non_blocking();
            m_on_accept(std::move(accepted_socket));
        }
    }

    /**
     * @brief Runs accept() if this socket isn't closed yet.
     * @return true if still open and accept() ran, false if closed.
     */
    bool resume() {
        // Closed listening socket never resumes.
        if (m_closed) {
            core::logger::warning("io/socket", "endpoint {} closed, cannot resume",
                                  m_socket.get_endpoint().to_string());
            return false;
        }
        // Still open — try accepting a connection.
        accept();
        return true;
    }

    /**
     * @brief Gets this handler's display name for the controller.
     * @return the fixed string `"ServerBaseSocket - Sync"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "ServerBaseSocket - Sync"; }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — resume()s and either reschedules or releases based on whether it's still open.
     * @return the per-execution work callable.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            // Still open — keep accepting and stay scheduled. Closed — release for good.
            if (resume()) {
                shared::this_handler::shedule();
            } else {
                shared::this_handler::release();
            }
        };
    }

    /**
     * @brief Builds the cleanup callback for release — closes the underlying listening socket.
     * @return the release callback.
     */
    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_socket.sync_close(); };
    }


    /**
     * @brief Gets the listening socket's fd.
     * @return the fd, read-only.
     */
    [[nodiscard]] const socket::SOCKET &get_fd() const noexcept { return m_socket.get().get_fd(); }
    /**
     * @brief Gets the listening socket's fd.
     * @return the fd, mutable.
     */
    socket::SOCKET &get_fd() noexcept { return m_socket.get().get_fd(); }
    /**
     * @brief Gets the endpoint this socket is bound/listening on.
     * @return the bound endpoint.
     */
    [[nodiscard]] socket::Endpoint get_endpoint() const noexcept { return m_socket.get_endpoint(); }
    /**
     * @brief Gets the endpoint this socket is bound/listening on.
     * @return the bound endpoint.
     */
    socket::Endpoint get_endpoint() noexcept { return m_socket.get_endpoint(); }
    /**
     * @brief Gets the endpoint most recently accepted/received on this socket.
     * @return the received endpoint.
     */
    socket::Endpoint get_recived_endpoint() noexcept { return m_socket.get_recived_endpoint(); }

  private:
    socket::Socket<Protocol, true> m_socket;
    OnAcceptCallback m_on_accept{nullptr};
    bool m_closed{false};
};


template <socket::Protocol Protocol>
class ClientBaseSocket : public shared::HandlerBase {
  public:
    /**
     * @brief Builds a client-side base socket bound to `leverager`'s io context — doesn't
     * connect yet, that's on start()/build().
     * @param end the endpoint to eventually connect to.
     * @param leverager the io_uring leverager backing this socket.
     */
    ClientBaseSocket(socket::Endpoint end, Leverager &leverager)
        : m_socket{socket::Socket<Protocol, true>{std::move(end), std::ref(leverager)}} {};

    /**
     * @brief Default dtor — no special teardown, on_released() handles socket close.
     */
    ~ClientBaseSocket() override = default;

    /**
     * @brief Copy ctor deleted — sockets aren't copyable.
     */
    ClientBaseSocket(const ClientBaseSocket &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    ClientBaseSocket &operator=(const ClientBaseSocket &) = delete;
    /**
     * @brief Move ctor — steals the underlying socket and closed flag off `other`.
     * @param other the socket to move from.
     */
    ClientBaseSocket(ClientBaseSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_closed{other.m_closed} {}
    /**
     * @brief Move assignment — self-assignment guarded, steals `other`'s state.
     * @param other the socket to move from.
     * @return `*this`.
     */
    ClientBaseSocket &operator=(ClientBaseSocket &&other) noexcept {
        // Guard self-assignment, then steal `other`'s state.
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_closed = other.m_closed;
        }
        return *this;
    };

    /**
     * @brief Kicks off start() unconditionally, no cap — no callback validation needed on the
     * client side since there's no accept callback to wait on.
     */
    void build() & { start(); }

    /**
     * @brief Runs the actual client bring-up sequence: registers the h2 ALPN proto, loads a TLS
     * cert/key pair, then attempts a sync connect. Only flips non-blocking on success.
     * @warning If `sync_connect()` doesn't come back VALID, this logs an error and returns —
     * silently. The socket's left in whatever half-connected state `sync_connect()` left it in,
     * `set_non_blocking()` never runs, and there's no exception or return value telling the
     * caller it failed. Check the logs, not the return type, because there isn't one.
     */
    void start() {
        // Advertise h2 and load the TLS cert/key pair before attempting to connect.
        m_socket.add_alpn_proto("h2");
        m_socket.generate_certificate("./server.crt", "server.key");
        m_socket.load_certificate("./server.crt", "server.key");
        // Attempt the sync connect — bail (silently, per the warning above) if it didn't land.
        auto connect_status = m_socket.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            core::logger::error("io/flow/client", "connect to {} failed",
                                m_socket.get_endpoint().to_string());
            return;
        }
        // Connected — only now flip non-blocking.
        m_socket.set_non_blocking();
    }

    /**
     * @brief Marks this socket closed.
     */
    void set_closed() { m_closed = true; }

    /**
     * @brief Gets the underlying socket's fd.
     * @return the fd, read-only.
     */
    [[nodiscard]] const socket::SOCKET &get_fd() const noexcept { return m_socket.get().get_fd(); }
    /**
     * @brief Gets the underlying socket's fd.
     * @return the fd, mutable.
     */
    socket::SOCKET &get_fd() noexcept { return m_socket.get().get_fd(); }
    /**
     * @brief Gets the endpoint this socket connects to.
     * @return the target endpoint.
     */
    [[nodiscard]] socket::Endpoint get_endpoint() const noexcept { return m_socket.get_endpoint(); }
    /**
     * @brief Gets the endpoint this socket connects to.
     * @return the target endpoint.
     */
    socket::Endpoint get_endpoint() noexcept { return m_socket.get_endpoint(); }
    /**
     * @brief Gets the endpoint most recently received on this socket.
     * @return the received endpoint.
     */
    socket::Endpoint get_recived_endpoint() noexcept { return m_socket.get_recived_endpoint(); }

  private:
    socket::Socket<Protocol, true> m_socket;
    bool m_closed{false};
};

template <socket::Protocol Protocol>
class ConnectorSocket : public shared::HandlerBase {
  public:
    using OnHandshakeComplete = std::function<void(socket::Socket<Protocol>)>;

    /**
     * @brief Builds a connector wrapping an already-accepted/connected socket that still needs
     * its TLS handshake driven to completion.
     * @param socket the raw socket to handshake over.
     * @param on_success invoked with the handshaked socket once the handshake lands.
     */
    ConnectorSocket(socket::Socket<Protocol> socket, OnHandshakeComplete on_success)
        : m_socket{std::move(socket)}, m_on_success{std::move(on_success)} {};

    /**
     * @brief Default dtor — no special teardown, on_released() handles socket close.
     */
    ~ConnectorSocket() override = default;

    /**
     * @brief Copy ctor deleted — sockets aren't copyable.
     */
    ConnectorSocket(const ConnectorSocket &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    ConnectorSocket &operator=(const ConnectorSocket &) = delete;
    /**
     * @brief Move ctor — steals the socket, success callback, and self-ownership pointer off
     * `other`.
     * @param other the connector to move from.
     */
    ConnectorSocket(ConnectorSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_on_success{std::move(other.m_on_success)},
          m_insane{std::move(other.m_insane)} {}
    /**
     * @brief Move assignment — self-assignment guarded, steals `other`'s state.
     * @param other the connector to move from.
     * @return `*this`.
     */
    ConnectorSocket &operator=(ConnectorSocket &&other) noexcept {
        // Guard self-assignment, then steal `other`'s socket, callback, and self-ownership state.
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_on_success = std::move(other.m_on_success);
            m_insane = std::move(other.m_insane);
        }
        return *this;
    };

    enum class ConnectResult : std::uint8_t { SUCCESS, ERROR, IN_PROGRESS };

    // Controll if should reshedule
    /**
     * @brief Drives one step of the sync TLS handshake — on success, hands the now-encrypted
     * socket off to `m_on_success` and moves it out of `m_socket`; on hard failure, just reports
     * it; otherwise signals the caller to keep pumping.
     * @return `SUCCESS` once the handshake completes and `m_on_success` has fired, `ERROR` on a
     * hard handshake failure, `IN_PROGRESS` if it needs another pass.
     */
    ConnectResult handshake() {
        // One step of the sync handshake — dispatch on where it landed this pass.
        socket::SocketStatus status = m_socket.sync_handshake();
        if (status.is_valid()) {
            // Done — hand the now-encrypted socket off and report success.
            core::logger::info("io/connector", "fd {} handshake ok", get_fd());
            m_on_success(std::move(m_socket));
            return ConnectResult::SUCCESS;
        }
        if (status.is_errored()) {
            // Hard failure — nothing more to try.
            core::logger::warning("io/connector", "fd {} handshake failed", get_fd());
            return ConnectResult::ERROR;
        }

        // Neither valid nor errored yet — needs another pass.
        core::logger::debug("io/connector", "fd {} handshake...", get_fd());
        return ConnectResult::IN_PROGRESS;
    }

    /**
     * @brief Gets this handler's display name for the controller.
     * @return the fixed string `"ConnectorSocket - Sync"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "ConnectorSocket - Sync"; }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — keeps pumping handshake() while IN_PROGRESS, releases on ERROR, and on SUCCESS drops the
     * self-ownership pointer (see `m_insane`/what_this_is_me()) so this connector can finally die.
     * @return the per-execution work callable.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            // Pump one handshake step, then branch on how it went.
            auto result = handshake();
            if (result == ConnectResult::IN_PROGRESS) {
                // Not done — stay scheduled for another pass.
                shared::this_handler::shedule();
                return;
            }
            if (result == ConnectResult::ERROR) {
                // Hard failure — release the handler.
                shared::this_handler::release();
                return;
            }

            // SUCCESS falls through here — drop the self-ownership pointer now that
            // m_on_success has already fired.
            m_insane.reset();
        };
    }

    /**
     * @brief Builds the cleanup callback for release — closes the underlying socket.
     * @return the release callback.
     */
    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_socket.sync_close(); };
    }

    /**
     * @brief Hands this connector its own `unique_ptr` so it can keep itself alive across the
     * multi-step handshake instead of getting destroyed the moment the local variable that
     * created it goes out of scope.
     * @warning This is a self-ownership pattern — `m_insane` holds a `unique_ptr<ConnectorSocket>`
     * pointing at `this`, so `this` object is keeping itself alive. `on_execute()`'s SUCCESS
     * branch calls `m_insane.reset()` as its last action, which is effectively a "delete this"
     * from inside a member function — technically fine here since nothing touches `this`
     * afterward, but a real footgun if that ever changes. Worse: the ERROR branch never resets
     * `m_insane` at all, it just releases the handler and returns — the self-owned
     * ConnectorSocket (and its `unique_ptr` to itself) never gets freed on a failed handshake, so
     * every failed connection permanently leaks one ConnectorSocket. No cap, that's a real leak,
     * not just vibes.
     * @param insane the unique_ptr to this same object, transferred in for self-ownership.
     */
    void what_this_is_me(std::unique_ptr<ConnectorSocket<Protocol>> insane) {
        m_insane = std::move(insane);
    }

    /**
     * @brief Gets the underlying socket's fd.
     * @return the fd, read-only.
     */
    [[nodiscard]] const socket::SOCKET &get_fd() const noexcept { return m_socket.get_fd(); }
    /**
     * @brief Gets the underlying socket's fd.
     * @return the fd, mutable.
     */
    socket::SOCKET &get_fd() noexcept { return m_socket.get_fd(); }

  private:
    socket::Socket<Protocol> m_socket;
    OnHandshakeComplete m_on_success;
    std::unique_ptr<ConnectorSocket<Protocol>> m_insane{nullptr};
};

template <socket::Protocol Protocol>
class WorkerSocket {
  public:
    /**
     * @brief Builds a WorkerSocket with no callbacks wired up — sender/receiver both start
     * blank, needs add_on_*() + build() before it's usable.
     * @param socket the connected socket this worker reads/writes.
     */
    WorkerSocket(socket::Socket<Protocol> socket)
        : m_socket{std::move(socket)}, m_sender{m_socket}, m_receiver{m_socket} {}

    /**
     * @brief Builds a WorkerSocket with send/receive error callbacks wired, but still needs a
     * read callback via add_on_read() + build() before it's fully usable.
     * @param socket the connected socket this worker reads/writes.
     * @param on_send_error invoked with `(fd, error)` on a send failure.
     * @param on_receive_error invoked with `(fd, error)` on a receive failure.
     */
    WorkerSocket(socket::Socket<Protocol> socket, shared::ErrorCallback on_send_error,
                 shared::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket, std::move(on_send_error)},
          m_receiver{m_socket, std::move(on_receive_error)} {}

    /**
     * @brief Builds a WorkerSocket fully wired up with read + both error callbacks and calls
     * build() immediately — ready to go once this constructor returns.
     * @param socket the connected socket this worker reads/writes.
     * @param on_read invoked with the buffer view on every completed read.
     * @param on_send_error invoked with `(fd, error)` on a send failure.
     * @param on_receive_error invoked with `(fd, error)` on a receive failure.
     */
    WorkerSocket(socket::Socket<Protocol> socket, shared::ReadCallback on_read,
                 shared::ErrorCallback on_send_error, shared::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket, std::move(on_send_error)},
          m_receiver{m_socket, std::move(on_read), std::move(on_receive_error)} {
        // Every callback's already wired via the init list — just validate/finalize both halves.
        build();
    }

    /**
     * @brief Closes this worker's socket via close() on the way out — teardown's guaranteed even
     * if nobody remembered to call close() manually.
     */
    ~WorkerSocket() { close(); }

    /**
     * @brief Copy ctor deleted — holds live sender/receiver state bound to one socket.
     */
    WorkerSocket(const WorkerSocket &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    WorkerSocket &operator=(const WorkerSocket &) = delete;
    /**
     * @brief Move ctor deleted — `m_sender`/`m_receiver` hold references straight into
     * `m_socket`, so moving this object would leave those references pointing at the old,
     * moved-from socket's address instead of the new one. Deleted to dodge that whole mess.
     */
    WorkerSocket(WorkerSocket &&other) = delete;
    /**
     * @brief Move assignment deleted, same internal-reference reasoning as the move ctor.
     */
    WorkerSocket &operator=(WorkerSocket &&other) = delete;

    /**
     * @brief Wires (or replaces) the receiver's read callback.
     * @param on_read the new read callback.
     */
    void add_on_read(shared::ReadCallback on_read) & { m_receiver.add_on_read(std::move(on_read)); }

    /**
     * @brief Wires (or replaces) the sender's error callback.
     * @param on_send_error the new send-error callback.
     */
    void add_on_send_error(shared::ErrorCallback on_send_error) & {
        m_sender.add_on_error(std::move(on_send_error));
    }

    /**
     * @brief Wires (or replaces) the receiver's error callback.
     * @param on_receive_error the new receive-error callback.
     */
    void add_on_receive_error(shared::ErrorCallback on_receive_error) & {
        m_receiver.add_on_error(std::move(on_receive_error));
    }

    /**
     * @brief Validates and finalizes both the sender and receiver.
     * @throws std::runtime_error if either the sender or receiver is missing a required callback
     * — see `Sender::build()`/`Receiver::build()`.
     */
    void build() & {
        // Finalize both halves — either can throw if it's missing a required callback.
        m_sender.build();
        m_receiver.build();
    }


    /**
     * @brief Registers this worker's sender and receiver against a controller so they actually
     * start getting scheduled.
     * @tparam Controller the controller type, must satisfy `shared::HandlerController`.
     * @param controller the controller to register against.
     */
    template <shared::HandlerController Controller>
    void start(Controller &controller) {
        // Register both halves against the controller so they actually get scheduled.
        m_sender.template create<Controller>(controller);
        m_receiver.template create<Controller>(controller);
    }

    /**
     * @brief Tears this worker down — marks both sender and receiver closed, then closes the
     * underlying socket. Bet, that's the whole shutdown sequence.
     */
    void close() {
        // Mark both halves closed before actually tearing down the socket underneath them.
        core::logger::debug("io/worker", "fd {} closed", get_fd());
        m_sender.set_closed();
        m_receiver.set_closed();
        m_socket.sync_close();
    }

    /**
     * @brief Gets the sender bound to this worker's socket.
     * @return the sender, read-only.
     */
    [[nodiscard]] const Sender<socket::Socket<Protocol>, socket::SocketStatus> &
    get_sender() const noexcept {
        return m_sender;
    }
    /**
     * @brief Gets the sender bound to this worker's socket.
     * @return the sender, mutable.
     */
    Sender<socket::Socket<Protocol>, socket::SocketStatus> &get_sender() { return m_sender; }
    /**
     * @brief Gets the receiver bound to this worker's socket.
     * @return the receiver, read-only.
     */
    [[nodiscard]] const Receiver<socket::Socket<Protocol>, socket::SocketStatus> &
    get_receiver() const noexcept {
        return m_receiver;
    }
    /**
     * @brief Gets the receiver bound to this worker's socket.
     * @return the receiver, mutable.
     */
    Receiver<socket::Socket<Protocol>, socket::SocketStatus> &get_receiver() noexcept {
        return m_receiver;
    }
    /**
     * @brief Gets the underlying socket's fd.
     * @return the fd, read-only.
     */
    [[nodiscard]] const socket::SOCKET &get_fd() const noexcept { return m_socket.get_fd(); }
    /**
     * @brief Gets the underlying socket's fd.
     * @return the fd, mutable.
     */
    socket::SOCKET &get_fd() noexcept { return m_socket.get_fd(); }

  private:
    socket::Socket<Protocol> m_socket;
    Sender<socket::Socket<Protocol>, socket::SocketStatus> m_sender;
    Receiver<socket::Socket<Protocol>, socket::SocketStatus> m_receiver;
};

// Wrapper that connects the base cocket to the worker and manages the types for the thread model.
template <shared::HandlerController Controller, socket::Protocol Protocol>
class ServerFlowSocket {
  public:
    using ConnectionEstablishedCallback =
        std::move_only_function<shared::ReadCallback(shared::SendCallback, shared::CloseCallback)>;

    /**
     * @brief Builds a ServerFlowSocket wrapping a not-yet-listening base socket — nothing's
     * actually started until build().
     * @param end the endpoint to eventually bind and listen on.
     * @param leverager the io_uring leverager backing the base socket.
     * @param controller the controller every accepted worker/connector gets registered against.
     */
    ServerFlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller)
        : m_base_socket{std::move(end), leverager}, m_leverager{leverager},
          m_controller{controller}, m_on_established{nullptr} {};

    /**
     * @brief Marks the base socket closed and closes out every still-live worker in `m_workers`
     * — makes sure nothing's left dangling when this flow socket goes away.
     */
    ~ServerFlowSocket() {
        // Stop accepting new connections first, then walk and close out every worker still
        // hanging around in the map. Destructors are implicitly noexcept, and logging/close()
        // could in theory throw (formatting, allocation) — an escaped exception here would
        // terminate the process, so swallow and best-effort the rest of the teardown instead.
        try {
            m_base_socket.set_closed();
            core::logger::debug("io/flow", "closing base socket {}",
                                m_base_socket.get_endpoint().to_string());
            for (auto &[fd, worker] : m_workers) {
                worker->close();
            }
        } catch (...) {
            try {
                core::logger::error("io/flow", "exception during ~ServerFlowSocket teardown");
            } catch (...) {  // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
            }
        }
    }

    /**
     * @brief Copy ctor deleted — this thing owns live sockets and a worker map, no copying that.
     */
    ServerFlowSocket(const ServerFlowSocket &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    ServerFlowSocket &operator=(const ServerFlowSocket &) = delete;
    /**
     * @brief Move ctor — steals the base socket, leverager/controller refs, worker map, and
     * established callback off `other`.
     * @param other the flow socket to move from.
     */
    ServerFlowSocket(ServerFlowSocket &&other) noexcept
        : m_base_socket{std::move(other.m_base_socket)}, m_leverager{other.m_leverager},
          m_controller{std::move(other.m_controller)}, m_workers{std::move(other.m_workers)},
          m_on_established(std::move(other.m_on_established)) {}
    /**
     * @brief Move assignment — self-assignment guarded, steals `other`'s state.
     * @param other the flow socket to move from.
     * @return `*this`.
     */
    ServerFlowSocket &operator=(ServerFlowSocket &&other) noexcept {
        // Guard self-assignment, then steal every bit of `other`'s state.
        if (this != &other) {
            m_base_socket = std::move(other.m_base_socket);
            m_leverager = other.m_leverager;
            m_controller = std::move(other.m_controller);
            m_workers = std::move(other.m_workers);
            m_on_established = std::move(other.m_on_established);
        }
        return *this;
    };


    /**
     * @brief Wires the callback invoked once a connection finishes its handshake and gets a
     * worker spun up for it — this is what connects application-level read handling to a live
     * worker.
     * @param established the connection-established callback.
     */
    void add_on_accept(ConnectionEstablishedCallback established) & {
        m_on_established = std::move(established);
    }

    /**
     * @brief Validates the established callback is set, wires up the base socket's accept
     * pipeline via helper(), then starts it.
     * @throws std::runtime_error if the established callback hasn't been set yet.
     */
    void build() & {
        // Established callback's mandatory — nothing to hand new workers off to otherwise.
        if (!m_on_established) {
            throw std::runtime_error(
                "ConnectionEstablished callback must be set before building the ServerFlowSocket");
        }
        // Wire the accept pipeline first, then actually start accepting.
        helper();
        start();
    }

    /**
     * @brief Registers the base socket against the controller so it actually starts accepting.
     */
    void start() {
        core::logger::debug("io/flow", "base socket {} start",
                            m_base_socket.get_endpoint().to_string());
        m_base_socket.template create<Controller>(m_controller);
    }

    /**
     * @brief Looks up the worker for `fd` and, if found, hands back a callback that sends
     * straight through its sender.
     * @param fd the socket fd to look up a worker for.
     * @return a send callback bound to that worker's sender, or `nullptr` if no worker exists
     * for `fd`.
     */
    shared::SendCallback on_send(socket::SOCKET socket_fd) {
        // Look up the worker for this fd — no worker means no send path.
        auto value = m_workers.find(socket_fd);
        if (value) {
            return [sender = &value->get_sender()](utils::buffering::BufferNode &&node) {
                sender->send(std::move(node));
            };
        }
        return nullptr;
    }

  private:
    /**
     * @brief Wires the base socket's accept callback: on every accepted connection, spins up a
     * `ConnectorSocket` to drive the TLS handshake, and on handshake success builds a
     * `WorkerSocket`, invokes `m_on_established` to get the application read callback, registers
     * the worker against the controller, and stashes it in `m_workers`.
     * @warning The nested lambdas here capture `this`, `worker` (a `shared_ptr`), and reach back
     * into `m_workers` from deep inside handshake/read/error callbacks. If this `ServerFlowSocket`
     * gets destroyed while a handshake or worker is still mid-flight, those captured `this`
     * pointers dangle — same class of footgun as the raw-`this` captures in the async
     * Sender/Receiver `arm_write()`/`arm_read()`. The `ConnectorSocket` self-ownership trick (see
     * `ConnectorSocket::what_this_is_me()`) keeps the connector itself alive, but doesn't protect
     * this `ServerFlowSocket` from an early death.
     */
    void helper() {
        // Wire what happens on every accepted raw connection.
        m_base_socket.add_on_accept([this](socket::Socket<Protocol> accepted_socket) mutable {
            core::logger::debug("io/flow", "accepted {} from {}",
                                m_base_socket.get_endpoint().to_string(),
                                accepted_socket.get_endpoint().to_string());
            // Captured before the move below — accepted_socket is read again afterward (for the
            // "handshake start" log), and reading a moved-from socket's fd would be a genuine
            // use-after-move.
            const auto ACCEPTED_FD = accepted_socket.get_fd();
            // Spin up a connector to drive the TLS handshake to completion for this socket.
            auto connector = std::make_unique<ConnectorSocket<Protocol>>(
                std::move(accepted_socket),
                [this](socket::Socket<Protocol> encrypted_socket) mutable {
                    // Handshake landed — build the worker, wiring send/receive error callbacks
                    // that drop the worker out of the map on a fatal I/O error.
                    auto worker = std::make_shared<WorkerSocket<Protocol>>(
                        std::move(encrypted_socket),
                        [this](socket::SOCKET socket_fd, int err) {
                            m_workers.erase(socket_fd);
                            core::logger::error("io/flow/server", "send error on socket {}: {}",
                                                socket_fd, err);
                        },
                        [this](socket::SOCKET socket_fd, int err) {
                            m_workers.erase(socket_fd);
                            core::logger::error("io/flow/server", "receive error on socket {}: {}",
                                                socket_fd, err);
                        });

                    // Ask the application layer for the actual read handler, handing it a send
                    // callback bound to this worker and a close callback that drops it from the map.
                    auto read_calback = m_on_established(
                        [worker](utils::buffering::BufferNode &&node) {
                            worker->get_sender().send(std::move(node));
                        },
                        [this, worker]() {
                            core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                            m_workers.erase(worker->get_fd());
                        });

                    // Wire the read callback, finalize the worker, then register it against the
                    // controller so it actually gets scheduled.
                    worker->add_on_read(std::move(read_calback));
                    worker->build();

                    worker->template start<Controller>(m_controller);

                    core::logger::debug("io/flow", "fd {} connected under {}", worker->get_fd(),
                                        m_base_socket.get_endpoint().to_string());

                    // Stash it so on_send()/the destructor can find it later.
                    m_workers.insert(worker->get_fd(), std::move(worker));
                });

            // Register the connector against the controller and hand it its own unique_ptr so it
            // stays alive across the multi-step handshake.
            core::logger::debug("io/flow", "fd {} handshake start", ACCEPTED_FD);
            connector->template create<Controller>(m_controller);
            connector->what_this_is_me(std::move(connector));
        });

        // Accept callback's wired — finalize the base socket itself.
        core::logger::debug("io/flow", "base socket {} start",
                            m_base_socket.get_endpoint().to_string());
        m_base_socket.build();
    }

    ServerBaseSocket<Protocol> m_base_socket;
    std::reference_wrapper<Leverager> m_leverager;
    std::reference_wrapper<Controller> m_controller;
    hashmap::swiss::SwissHashMap<socket::SOCKET, std::shared_ptr<WorkerSocket<Protocol>>> m_workers;
    ConnectionEstablishedCallback m_on_established;
};

template <shared::HandlerController Controller, socket::Protocol Protocol>
class ClientFlowSocket {
  public:
    using ConnectionEstablishedCallback =
        std::move_only_function<shared::ReadCallback(shared::SendCallback, shared::CloseCallback)>;

    /**
     * @brief Builds a ClientFlowSocket targeting `end` — doesn't actually connect yet, that's on
     * build()/helper().
     * @param end the endpoint to eventually connect to.
     * @param leverager the io_uring leverager backing the connection.
     * @param controller the controller the connector/worker get registered against.
     * @param verify_peer forwarded straight to `Socket::set_verify_peer()` before connecting —
     * `false` when connecting to a self-signed/dev cert not in the system trust store.
     */
    ClientFlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller,
                     bool verify_peer = true)
        : m_endpoint{std::move(end)}, m_leverager{leverager}, m_controller{controller},
          m_on_established{nullptr}, m_verify_peer{verify_peer} {}

    /// @brief Closes out the live worker, if any.
    ~ClientFlowSocket() {
        // m_worker is a single optional, not a range — close it directly if it's there.
        if (m_worker) {
            (*m_worker)->close();
        }
    }

    /**
     * @brief Copy ctor deleted — owns a live socket/worker, no copying that.
     */
    ClientFlowSocket(const ClientFlowSocket &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    ClientFlowSocket &operator=(const ClientFlowSocket &) = delete;
    /**
     * @brief Move ctor — steals the endpoint, leverager/controller refs, worker, and established
     * callback off `other`.
     * @param other the flow socket to move from.
     */
    ClientFlowSocket(ClientFlowSocket &&other) noexcept
        : m_endpoint{std::move(other.m_endpoint)}, m_leverager{other.m_leverager},
          m_controller{std::move(other.m_controller)}, m_worker{std::move(other.m_worker)},
          m_on_established{std::move(other.m_on_established)},
          m_verify_peer{other.m_verify_peer} {}
    /**
     * @brief Move assignment — self-assignment guarded, steals `other`'s state.
     * @param other the flow socket to move from.
     * @return `*this`.
     */
    ClientFlowSocket &operator=(ClientFlowSocket &&other) noexcept {
        // Guard self-assignment, then steal every bit of `other`'s state.
        if (this != &other) {
            m_endpoint = std::move(other.m_endpoint);
            m_leverager = other.m_leverager;
            m_controller = std::move(other.m_controller);
            m_worker = std::move(other.m_worker);
            m_on_established = std::move(other.m_on_established);
            m_verify_peer = other.m_verify_peer;
        }
        return *this;
    }

    /**
     * @brief Wires the callback invoked once the handshake completes and a worker gets spun up.
     * @param established the connection-established callback.
     */
    void add_on_accept(ConnectionEstablishedCallback established) & {
        m_on_established = std::move(established);
    }

    /**
     * @brief Validates the established callback is set, then drives the connect/handshake
     * sequence via helper().
     * @throws std::runtime_error if the established callback hasn't been set yet.
     */
    void build() & {
        // Established callback's mandatory before the connect/handshake sequence can hand
        // anything off.
        if (!m_on_established) {
            throw std::runtime_error(
                "ConnectionEstablished callback must be set before building the ClientFlowSocket");
        }
        // Callback confirmed — drive the actual connect/handshake/worker-setup sequence.
        helper();
    }

    /**
     * @brief Hands back a callback that sends through the live worker's sender, if there is one.
     * @return a send callback bound to the worker's sender, or `nullptr` if no worker exists yet.
     */
    shared::SendCallback on_send() {
        // Check has_value() first, only then dereference — an empty m_worker just falls
        // through to nullptr instead of throwing std::bad_optional_access.
        if (m_worker.has_value()) {
            auto &value = *m_worker;
            return [sender = &value->get_sender()](utils::buffering::BufferNode &&node) {
                sender->send(std::move(node));
            };
        }
        return nullptr;
    }

  private:
    /**
     * @brief Builds a raw socket for `endpoint`, registers h2 ALPN, and attempts a sync connect
     * — flips non-blocking only once connected.
     * @note No cert/key gets loaded here — that's a server-side concern (presenting a cert to
     * clients). `sync_connect()` handles the client-side TLS setup itself, internally, via
     * `setup_tls()` once the raw TCP connect lands (ALPN + peer verification against the
     * default trust store, no cert of its own to load).
     * @param endpoint the target endpoint to connect to.
     * @param verify_peer forwarded to `Socket::set_verify_peer()` before connecting.
     * @return a socket connected and flipped non-blocking on success.
     * @throws std::runtime_error if the sync connect fails — `helper()` uses the returned
     * socket unconditionally with no failure check, so a failed connect can't return a
     * half-usable socket here.
     */
    static socket::Socket<Protocol> create_socket(socket::Endpoint endpoint, bool verify_peer) {
        // Build the raw socket and advertise h2 before doing anything else.
        socket::Socket<Protocol> socket{std::move(endpoint)};
        socket.add_alpn_proto("h2");
        socket.set_verify_peer(verify_peer);

        // Attempt the sync connect — throw on failure, since helper() below uses the
        // returned socket unconditionally with no failure check of its own.
        auto connect_status = socket.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            throw std::runtime_error(
                std::format("connect to {} failed", socket.get_endpoint().to_string()));
        }
        // Connected — flip non-blocking and hand the socket back.
        socket.set_non_blocking();
        return socket;
    }

    /**
     * @brief Drives the full client connect sequence: builds the socket via create_socket(),
     * spins up a `ConnectorSocket` for the TLS handshake, and on handshake success builds a
     * `WorkerSocket`, invokes `m_on_established` for the application read callback, registers the
     * worker against the controller, and stores it in `m_worker`.
     * @warning Same nested-lambda `this`-capture lifetime concern as
     * `ServerFlowSocket::helper()` — if this `ClientFlowSocket` dies while the handshake or
     * worker setup is still in flight, the captured `this` in these callbacks dangles.
     */
    void helper() {
        // Build and connect the raw socket first.
        core::logger::debug("io/flow", "base socket {} start", m_endpoint.to_string());

        auto socket = create_socket(m_endpoint, m_verify_peer);

        core::logger::debug("io/flow", "fd {} handshake start", socket.get_fd());

        // Spin up a connector to drive the TLS handshake for this socket.
        auto connector = std::make_unique<ConnectorSocket<Protocol>>(
            std::move(socket), [this](socket::Socket<Protocol> encrypted_socket) mutable {
                // Handshake landed — build the worker, wiring send/receive error callbacks that
                // drop this client's worker on a fatal I/O error.
                auto worker = std::make_shared<WorkerSocket<Protocol>>(
                    std::move(encrypted_socket),
                    [this](socket::SOCKET socket_fd, int err) {
                        m_worker.reset();
                        core::logger::error("io/flow/server", "send error on socket {}: {}",
                                            socket_fd, err);
                    },
                    [this](socket::SOCKET socket_fd, int err) {
                        m_worker.reset();
                        core::logger::error("io/flow/server", "receive error on socket {}: {}",
                                            socket_fd, err);
                    });

                // Ask the application layer for the read handler, handing it a send callback
                // bound to this worker and a close callback that resets m_worker.
                auto read_calback = m_on_established(
                    [worker](utils::buffering::BufferNode &&node) {
                        worker->get_sender().send(std::move(node));
                    },
                    [this, worker]() {
                        core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                        m_worker.reset();
                    });

                // Wire the read callback, finalize the worker, then register it against the
                // controller so it actually gets scheduled.
                worker->add_on_read(std::move(read_calback));
                worker->build();

                worker->template start<Controller>(m_controller);

                core::logger::debug("io/flow", "fd {} connected under {}", worker->get_fd(),
                                    m_endpoint.to_string());

                // Stash it so on_send()/the destructor can find it later.
                m_worker.emplace(std::move(worker));
            });


        // Register the connector against the controller and hand it its own unique_ptr so it
        // stays alive across the multi-step handshake.
        connector->template create<Controller>(m_controller);
        connector->what_this_is_me(std::move(connector));
    }

    socket::Endpoint m_endpoint;
    std::reference_wrapper<Leverager> m_leverager;
    std::reference_wrapper<Controller> m_controller;
    std::optional<std::shared_ptr<WorkerSocket<Protocol>>> m_worker;
    ConnectionEstablishedCallback m_on_established;
    bool m_verify_peer;
};

} // namespace io::base::flow::sync
