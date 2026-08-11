export module io_flow_socket:sync;

import std;
import core_events;
import core_logger;
import core_contract;
import io_base_socket;
import utils_buffering;
import io_base_leverage;
import io_flow_sender;
import io_flow_receiver;
import interfaces;
import shared;
import hashmap;
import utils_errno_translator;

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
            core::events::publish("io.socket.closed_cannot_resume",
                                  {{"endpoint", m_socket.get_endpoint().to_string()}});
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
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "ServerBaseSocket - Sync";
    }

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
            core::events::publish("io.flow.client.connect_failed",
                                  {{"endpoint", m_socket.get_endpoint().to_string()}});
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
     * @brief Move ctor — steals the socket, success callback, and release hook off `other`.
     * @param other the connector to move from.
     */
    ConnectorSocket(ConnectorSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_on_success{std::move(other.m_on_success)},
          m_on_released_extra{std::move(other.m_on_released_extra)} {}
    /**
     * @brief Move assignment — self-assignment guarded, steals `other`'s state.
     * @param other the connector to move from.
     * @return `*this`.
     */
    ConnectorSocket &operator=(ConnectorSocket &&other) noexcept {
        // Guard self-assignment, then steal `other`'s socket, callback, and release hook.
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_on_success = std::move(other.m_on_success);
            m_on_released_extra = std::move(other.m_on_released_extra);
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
            core::events::publish("io.connector.handshake_failed",
                                  {{"fd", std::to_string(get_fd())}});
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
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "ConnectorSocket - Sync";
    }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — keeps pumping handshake() while IN_PROGRESS; on both SUCCESS and ERROR it releases the
     * handler instead of tearing down directly here. release() only flags/reschedules the
     * contract, so the actual close (in on_released()) happens later, once the controller is
     * truly done with `this`.
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

            // Both SUCCESS and ERROR release the contract — release() is deferred (it just
            // flags and reschedules), so on_released() hasn't run yet and still needs `this`
            // alive. Without this, SUCCESS never released its contract slot at all — a smaller,
            // non-fatal leak (nothing spins on it, but it's never reclaimed either) left over
            // from before.
            shared::this_handler::release();
        };
    }

    /**
     * @brief Builds the cleanup callback for release — fires the "about to close" hook (see
     * `set_on_released()`), which is what actually removes this connector from whichever
     * container owns it (e.g. `ServerFlowSocket::m_pending_connectors`), destroying `this` in the
     * process. No explicit socket close here: `~ConnectorSocket()`/`~Socket()` already close the
     * underlying fd/SSL state automatically as part of that destruction, same as any other RAII
     * teardown — see `Socket::~Socket()`. On SUCCESS, `m_socket` was already moved out to
     * `m_on_success` in handshake() (left in its moved-from, `INVALID_SOCKET` state), so that
     * destructor-time close is a documented no-op there too.
     * @warning `m_on_released_extra()` is expected to destroy `this` (by dropping the owning
     * container's entry) — it must stay the last statement here, nothing may touch `this`
     * afterward.
     * @return the release callback.
     */
    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            if (m_on_released_extra) {
                m_on_released_extra();
            }
        };
    }

    /**
     * @brief Sets the callback that owns/removes this connector once its handshake is done —
     * fired from `on_released()` once the controller is finished with it. The callback is
     * expected to drop whatever container is holding this connector (e.g.
     * `ServerFlowSocket::m_pending_connectors.erase(fd)`), which destroys `this` as a side effect
     * and closes the socket via `~ConnectorSocket()`/`~Socket()`.
     * @param callback the callback to fire; replaces whatever was set before.
     */
    void set_on_released(std::function<void()> callback) {
        m_on_released_extra = std::move(callback);
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
    std::function<void()> m_on_released_extra;
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
     * @brief Tears the underlying socket down on destruction. By the time a worker is destroyed
     * (erased after drain_senders() waited for its contracts to go idle) both halves are already
     * closed and their contracts released, so this just closes the fd for real.
     */
    ~WorkerSocket() { m_socket.sync_close(); }

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
        // Register both halves against the controller so they actually get scheduled, and stash
        // the handles so we can release the contracts cleanly during shutdown/drain.
        m_sender_contract.emplace(m_sender.template create<Controller>(controller));
        m_receiver_contract.emplace(m_receiver.template create<Controller>(controller));
    }

    /**
     * @brief Tears this worker down — marks both sender and receiver closed, releases their
     * contracts, then closes the underlying socket.
     */
    void mark_close() {
        core::logger::debug("io/worker", "fd {} closing", get_fd());
        m_sender.set_closed();
        m_receiver.set_closed();
    }

    /**
     * @brief Whether both of this worker's contracts have gone idle/released — i.e. the final
     * closed passes have run (armed slot released). drain_senders() polls this before destroying
     * the worker (which closes the socket).
     * @return true if both contracts are released or idle.
     */
    [[nodiscard]] bool contracts_idle() const noexcept {
        auto done = [](const std::optional<core::contract::Contract<>> &contract) {
            return !contract.has_value() || contract->is_released() || contract->is_idle();
        };
        return done(m_sender_contract) && done(m_receiver_contract);
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
    std::optional<core::contract::Contract<>> m_sender_contract;
    std::optional<core::contract::Contract<>> m_receiver_contract;
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
        // hanging around in the map. Nothing here can actually throw: core::logger::*/
        // core::events::publish are noexcept end to end, and Endpoint::to_string() (the one
        // real throw site this used to guard) is noexcept now too — see its own doc.
        m_base_socket.set_closed();
        core::logger::debug("io/flow", "closing base socket {}",
                            m_base_socket.get_endpoint().to_string());
        for (auto &[fd, worker] : m_workers) {
            worker->mark_close();
        }
        close_pending_connectors();
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
          m_pending_connectors{std::move(other.m_pending_connectors)},
          m_on_established(std::move(other.m_on_established)),
          m_base_socket_contract{std::move(other.m_base_socket_contract)} {}
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
            m_pending_connectors = std::move(other.m_pending_connectors);
            m_on_established = std::move(other.m_on_established);
            m_base_socket_contract = std::move(other.m_base_socket_contract);
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
        m_base_socket_contract.emplace(m_base_socket.template create<Controller>(m_controller));
    }

    /**
     * @brief Stops accepting new connections — closes the listening socket, releases its contract,
     * and clears any connectors still mid-handshake.
     */
    void stop_accepting() {
        m_base_socket.set_closed();
        close_pending_connectors();
        if (m_base_socket_contract.has_value()) {
            m_base_socket_contract->release();
            m_base_socket_contract.reset();
        }
    }

    /**
     * @brief Stops arming reads on every live worker immediately. Existing outbound sends still
     * flush, but no more inbound data is read.
     */
    void stop_receiving() {
        // for (auto &[fd, worker] : m_workers) {
        //     worker->get_receiver().set_closed();
        // }
    }

    /**
     * @brief Returns how many live worker connections are still held.
     */
    [[nodiscard]] std::size_t active_connections() const noexcept { return m_workers.size(); }

    /**
     * @brief Force-closes every live worker. Used as a drain-timeout fallback.
     */
    void close_all_workers() {
        for (auto &[fd, worker] : m_workers) {
            worker->mark_close();
        }
    }

    /**
     * @brief Closes every live worker (flags both halves closed; the sender flushes on close),
     * waits until each worker's contracts have gone idle — meaning the receiver ran its final
     * closed arm_read() pass and released its armed buffer slot — then drops the workers.
     * @note No timeout, and MUST run while the contract pool is still scheduling, so those final
     * passes actually execute. Sockets are closed later in ~WorkerSocket(). Call after GOAWAY and
     * session drain.
     */
    void drain_senders() noexcept {
        std::vector<std::shared_ptr<WorkerSocket<Protocol>>> workers;
        workers.reserve(m_workers.size());
        for (auto &[fd, worker] : m_workers) {
            workers.push_back(worker);
        }

        // Flag every worker closed — kicks off the self-releasing final pass on both contracts.
        for (auto &worker : workers) {
            worker->mark_close();
        }

        // Block until every worker's contracts have released/idled.
        bool all_idle = false;
        while (!all_idle) {
            all_idle = true;
            for (const auto &worker : workers) {
                if (!worker->contracts_idle()) {
                    all_idle = false;
                    break;
                }
            }
            if (!all_idle) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // Drop from the map — ~WorkerSocket() closes each fd once its last ref is gone.
        for (auto &worker : workers) {
            m_workers.erase(worker->get_fd());
        }
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
     * into `m_workers`/`m_pending_connectors` from deep inside handshake/read/error callbacks. If
     * this `ServerFlowSocket` gets destroyed while a handshake or worker is still mid-flight,
     * those captured `this` pointers dangle — same class of footgun as the raw-`this` captures in
     * the async Sender/Receiver `arm_write()`/`arm_read()`. `m_pending_connectors` (which owns
     * every in-flight `ConnectorSocket` outright, see its own doc) keeps the connector itself
     * alive across the handshake, but doesn't protect this `ServerFlowSocket` from an early death.
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
                            core::logger::error("io/flow/server",
                                                "send error on socket {}: {} ({})", socket_fd, err,
                                                utils::ErrnoTranslator::describe_errno(err));
                            core::events::publish(
                                "io.flow.server.send_error",
                                {{"fd", std::to_string(socket_fd)},
                                 {"error_code", std::to_string(err)},
                                 {"error",
                                  std::string{utils::ErrnoTranslator::describe_errno(err)}}});
                        },
                        [this](socket::SOCKET socket_fd, int err) {
                            m_workers.erase(socket_fd);
                            core::logger::error("io/flow/server",
                                                "receive error on socket {}: {} ({})", socket_fd,
                                                err, utils::ErrnoTranslator::describe_errno(err));
                            core::events::publish(
                                "io.flow.server.receive_error",
                                {{"fd", std::to_string(socket_fd)},
                                 {"error_code", std::to_string(err)},
                                 {"error",
                                  std::string{utils::ErrnoTranslator::describe_errno(err)}}});
                        });

                    // Ask the application layer for the actual read handler, handing it a send
                    // callback bound to this worker and a close callback that drops it from the
                    // map.
                    auto read_calback = m_on_established(
                        [worker](utils::buffering::BufferNode &&node) {
                            worker->get_sender().send(std::move(node));
                        },
                        [this, worker]() {
                            core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                            worker->mark_close();
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

            // m_pending_connectors owns the connector outright for as long as its handshake
            // stays in flight — see close_pending_connectors() and the member's own doc.
            // set_on_released() erases this entry (destroying the connector, which closes its
            // socket via ~ConnectorSocket()/~Socket()) the instant the handshake actually
            // finishes, success or failure alike.
            connector->set_on_released(
                [this, ACCEPTED_FD]() { m_pending_connectors.erase(ACCEPTED_FD); });

            // Register the connector against the controller, then hand ownership to
            // m_pending_connectors so it stays alive across the multi-step handshake.
            core::logger::debug("io/flow", "fd {} handshake start", ACCEPTED_FD);
            connector->template create<Controller>(m_controller);
            m_pending_connectors.insert(ACCEPTED_FD, std::move(connector));
        });

        // Accept callback's wired — finalize the base socket itself.
        core::logger::debug("io/flow", "base socket {} start",
                            m_base_socket.get_endpoint().to_string());
        m_base_socket.build();
    }

    /**
     * @brief Closes out every connection still stuck mid-handshake in `m_pending_connectors`.
     * @note Plain `clear()` — destroying each entry's owned `ConnectorSocket` closes its socket
     * automatically via `~ConnectorSocket()`/`~Socket()`, no explicit close call needed. `clear()`
     * runs those destructors through its own internal sweep (not our own iteration), so there's
     * no risk of the on-released erase-hook (only fired from the controller-driven path, never
     * from plain destruction) invalidating anything mid-loop. Without this, a connection still
     * mid-TLS-handshake when shutdown hits never gets a chance to run its normal on_released()
     * path either (nothing's left to schedule it there), leaking its socket/SSL state —
     * confirmed live under LeakSanitizer.
     */
    void close_pending_connectors() { m_pending_connectors.clear(); }

    ServerBaseSocket<Protocol> m_base_socket;
    std::reference_wrapper<Leverager> m_leverager;
    std::reference_wrapper<Controller> m_controller;
    hashmap::swiss::SwissHashMap<socket::SOCKET, std::shared_ptr<WorkerSocket<Protocol>>> m_workers;
    hashmap::swiss::SwissHashMap<socket::SOCKET, std::unique_ptr<ConnectorSocket<Protocol>>>
        m_pending_connectors;
    ConnectionEstablishedCallback m_on_established;
    std::optional<core::contract::Contract<>> m_base_socket_contract;
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
            (*m_worker)->mark_close();
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
          m_pending_connector{std::move(other.m_pending_connector)},
          m_on_established{std::move(other.m_on_established)}, m_verify_peer{other.m_verify_peer} {}
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
            m_pending_connector = std::move(other.m_pending_connector);
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
     * @return success, or an error if the established callback hasn't been set yet, or if the
     * synchronous portion of the attempt (create_socket()) fails.
     */
    std::expected<void, std::string> build() & {
        // Established callback's mandatory before the connect/handshake sequence can hand
        // anything off.
        if (!m_on_established) {
            return std::unexpected(
                "ConnectionEstablished callback must be set before building the ClientFlowSocket");
        }
        // Callback confirmed — drive the actual connect/handshake/worker-setup sequence.
        return helper();
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

    /**
     * @brief True once a worker or a pending connector actually exists — something is in
     * flight. False means the flow is idle: either never started, or a previous attempt has
     * fully settled (succeeded and later dropped, or failed and given up).
     * @return whether this flow currently has a worker or a connector alive.
     */
    [[nodiscard]] bool is_running() const noexcept {
        return m_worker.has_value() || m_pending_connector.has_value();
    }

    /**
     * @brief Retries the connect/handshake sequence. If nothing is currently connected or in
     * flight, starts a fresh attempt immediately via helper(). If a handshake from a previous
     * attempt is still in flight, defers instead of racing it — sets m_retry so that attempt's
     * own set_on_released() (see helper()) starts the next attempt itself once it finishes,
     * which is the only place this can safely happen without tearing down a connector that a
     * pool thread might still be about to run.
     * @return success, or an error if already connected, or if the synchronous portion of a
     * fresh attempt (create_socket()) fails.
     */
    std::expected<void, std::string> retry() {
        if (m_worker.has_value()) {
            return std::unexpected("client worker already connected and running");
        }
        if (m_pending_connector.has_value()) {
            m_retry = true;
            return {};
        }
        return helper();
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
     * @return a socket connected and flipped non-blocking on success, or an error message if
     * the sync connect fails — `helper()` checks this before touching the socket, so a failed
     * connect can't produce a half-usable socket downstream.
     */
    static std::expected<socket::Socket<Protocol>, std::string>
    create_socket(socket::Endpoint endpoint, bool verify_peer) {
        // Build the raw socket and advertise h2 before doing anything else.
        socket::Socket<Protocol> socket{std::move(endpoint)};
        socket.add_alpn_proto("h2");
        socket.set_verify_peer(verify_peer);

        // Attempt the sync connect — helper() checks the result below instead of assuming
        // success.
        auto connect_status = socket.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            return std::unexpected(
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
     * @return success once the connector's registered and driving the handshake, or an error
     * if the synchronous connect (create_socket()) fails.
     */
    std::expected<void, std::string> helper() {
        // Build and connect the raw socket first.
        core::logger::debug("io/flow", "base socket {} start", m_endpoint.to_string());

        auto socket_result = create_socket(m_endpoint, m_verify_peer);
        if (!socket_result) {
            return std::unexpected(socket_result.error());
        }
        auto socket = std::move(*socket_result);

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
                        core::logger::error("io/flow/server", "send error on socket {}: {} ({})",
                                            socket_fd, err,
                                            utils::ErrnoTranslator::describe_errno(err));
                        core::events::publish(
                            "io.flow.server.send_error",
                            {{"fd", std::to_string(socket_fd)},
                             {"error_code", std::to_string(err)},
                             {"error", std::string{utils::ErrnoTranslator::describe_errno(err)}}});
                    },
                    [this](socket::SOCKET socket_fd, int err) {
                        m_worker.reset();
                        core::logger::error("io/flow/server", "receive error on socket {}: {} ({})",
                                            socket_fd, err,
                                            utils::ErrnoTranslator::describe_errno(err));
                        core::events::publish(
                            "io.flow.server.receive_error",
                            {{"fd", std::to_string(socket_fd)},
                             {"error_code", std::to_string(err)},
                             {"error", std::string{utils::ErrnoTranslator::describe_errno(err)}}});
                    });

                // Ask the application layer for the read handler, handing it a send callback
                // bound to this worker and a close callback that resets m_worker.
                auto read_calback = m_on_established(
                    [worker](utils::buffering::BufferNode &&node) {
                        worker->get_sender().send(std::move(node));
                    },
                    [this, worker]() {
                        core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                        worker->mark_close();
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

        connector->set_on_released([this]() {
            m_pending_connector.reset();
            if (m_retry && !m_worker.has_value()) {
                m_retry = false;
                auto result = helper();
                if (!result) {
                    core::logger::warning("io/flow", "retry connect failed: {}", result.error());
                }
            }
        });

        // Register the connector against the controller, then hand ownership to
        // m_pending_connector so it stays alive across the multi-step handshake.
        connector->template create<Controller>(m_controller);
        m_pending_connector.emplace(std::move(connector));
        return {};
    }

    socket::Endpoint m_endpoint;
    std::reference_wrapper<Leverager> m_leverager;
    std::reference_wrapper<Controller> m_controller;
    std::optional<std::shared_ptr<WorkerSocket<Protocol>>> m_worker;
    std::optional<std::unique_ptr<ConnectorSocket<Protocol>>> m_pending_connector;
    ConnectionEstablishedCallback m_on_established;
    bool m_verify_peer;
    bool m_retry{false};
};

} // namespace io::base::flow::sync
