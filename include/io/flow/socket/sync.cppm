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

    ServerBaseSocket(socket::Endpoint end, Leverager &leverager)
        : m_socket{socket::Socket<Protocol, true>{std::move(end), std::ref(leverager)}},
          m_on_accept{nullptr}, m_closed{false} {};

    ServerBaseSocket(const ServerBaseSocket &) = delete;
    ServerBaseSocket &operator=(const ServerBaseSocket &) = delete;
    ServerBaseSocket(ServerBaseSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_on_accept{std::move(other.m_on_accept)},
          m_closed{std::move(other.m_closed)} {}
    ServerBaseSocket &operator=(ServerBaseSocket &&other) noexcept {
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_on_accept = std::move(other.m_on_accept);
            m_closed = std::move(other.m_closed);
        }
        return *this;
    };

    void add_on_accept(OnAcceptCallback on_accept) & { m_on_accept = std::move(on_accept); }

    void build() & {
        if (!m_on_accept) {
            throw std::runtime_error(
                "OnAccept callback must be set before building the ServerBaseSocket");
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
        socket::Socket<Protocol> accepted_socket = m_socket.sync_accept();
        if (accepted_socket) {
            core::logger::debug("io/socket", "accepted {} from {}",
                                m_socket.get_endpoint().to_string(),
                                accepted_socket.get_endpoint().to_string());
            accepted_socket.set_non_blocking();
            m_on_accept(std::move(accepted_socket));
        }
    }

    bool resume() {
        if (m_closed) {
            core::logger::warning("io/socket", "endpoint {} closed, cannot resume",
                                  m_socket.get_endpoint().to_string());
            return false;
        }
        accept();
        return true;
    }

    std::string_view get_name() const noexcept override { return "ServerBaseSocket - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            if (resume()) {
                shared::this_handler::shedule();
            } else {
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_socket.sync_close(); };
    }


    const socket::SOCKET &get_fd() const noexcept { return m_socket.get().get_fd(); }
    socket::SOCKET &get_fd() noexcept { return m_socket.get().get_fd(); }
    const socket::Endpoint get_endpoint() const noexcept { return m_socket.get_endpoint(); }
    socket::Endpoint get_endpoint() noexcept { return m_socket.get_endpoint(); }
    socket::Endpoint get_recived_endpoint() noexcept { return m_socket.get_recived_endpoint(); }

  private:
    socket::Socket<Protocol, true> m_socket;
    OnAcceptCallback m_on_accept;
    bool m_closed;
};


template <socket::Protocol Protocol>
class ClientBaseSocket : public shared::HandlerBase {
  public:
    ClientBaseSocket(socket::Endpoint end, Leverager &leverager)
        : m_socket{socket::Socket<Protocol, true>{std::move(end), std::ref(leverager)}},
          m_closed{false} {};

    ClientBaseSocket(const ClientBaseSocket &) = delete;
    ClientBaseSocket &operator=(const ClientBaseSocket &) = delete;
    ClientBaseSocket(ClientBaseSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_closed{std::move(other.m_closed)} {}
    ClientBaseSocket &operator=(ClientBaseSocket &&other) noexcept {
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_closed = std::move(other.m_closed);
        }
        return *this;
    };

    void build() & { start(); }

    void start() {
        m_socket.add_alpn_proto("h2");
        m_socket.generate_certificate("./server.crt", "server.key");
        m_socket.load_certificate("./server.crt", "server.key");
        auto connect_status = m_socket.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            core::logger::error("io/flow/client", "connect to {} failed",
                                m_socket.get_endpoint().to_string());
            return;
        }
        m_socket.set_non_blocking();
    }

    void set_closed() { m_closed = true; }

    [[nodiscard]] const socket::SOCKET &get_fd() const noexcept { return m_socket.get().get_fd(); }
    socket::SOCKET &get_fd() noexcept { return m_socket.get().get_fd(); }
    [[nodiscard]] socket::Endpoint get_endpoint() const noexcept { return m_socket.get_endpoint(); }
    socket::Endpoint get_endpoint() noexcept { return m_socket.get_endpoint(); }
    socket::Endpoint get_recived_endpoint() noexcept { return m_socket.get_recived_endpoint(); }

  private:
    socket::Socket<Protocol, true> m_socket;
    bool m_closed;
};

template <socket::Protocol Protocol>
class ConnectorSocket : public shared::HandlerBase {
  public:
    using OnHandshakeComplete = std::function<void(socket::Socket<Protocol>)>;

    ConnectorSocket(socket::Socket<Protocol> socket, OnHandshakeComplete on_success)
        : m_socket{std::move(socket)}, m_on_success{std::move(on_success)}, m_insane{nullptr} {};

    ~ConnectorSocket() = default;

    ConnectorSocket(const ConnectorSocket &) = delete;
    ConnectorSocket &operator=(const ConnectorSocket &) = delete;
    ConnectorSocket(ConnectorSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_on_success{std::move(other.m_on_success)},
          m_insane{std::move(other.m_insane)} {}
    ConnectorSocket &operator=(ConnectorSocket &&other) noexcept {
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_on_success = std::move(other.m_on_success);
            m_insane = std::move(other.m_insane);
        }
        return *this;
    };

    enum class ConnectResult : std::uint8_t { SUCCESS, ERROR, IN_PROGRESS };

    // Controll if should reshedule
    ConnectResult handshake() {
        socket::SocketStatus status = m_socket.sync_handshake();
        if (status.is_valid()) {
            core::logger::info("io/connector", "fd {} handshake ok", get_fd());
            m_on_success(std::move(m_socket));
            return ConnectResult::SUCCESS;
        } else if (status.is_errored()) {
            core::logger::warning("io/connector", "fd {} handshake failed", get_fd());
            return ConnectResult::ERROR;
        }

        core::logger::debug("io/connector", "fd {} handshake...", get_fd());
        return ConnectResult::IN_PROGRESS;
    }

    std::string_view get_name() const noexcept override { return "ConnectorSocket - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            auto result = handshake();
            if (result == ConnectResult::IN_PROGRESS) {
                shared::this_handler::shedule();
                return;
            } else if (result == ConnectResult::ERROR) {
                shared::this_handler::release();
                return;
            }

            m_insane.reset();
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_socket.sync_close(); };
    }

    void what_this_is_me(std::unique_ptr<ConnectorSocket<Protocol>> insane) {
        m_insane = std::move(insane);
    }

    const socket::SOCKET &get_fd() const noexcept { return m_socket.get_fd(); }
    socket::SOCKET &get_fd() noexcept { return m_socket.get_fd(); }

  private:
    socket::Socket<Protocol> m_socket;
    OnHandshakeComplete m_on_success;
    std::unique_ptr<ConnectorSocket<Protocol>> m_insane;
};

template <socket::Protocol Protocol>
class WorkerSocket {
  public:
    WorkerSocket(socket::Socket<Protocol> socket)
        : m_socket{std::move(socket)}, m_sender{m_socket}, m_receiver{m_socket} {}

    WorkerSocket(socket::Socket<Protocol> socket, shared::ErrorCallback on_send_error,
                 shared::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket, std::move(on_send_error)},
          m_receiver{m_socket, std::move(on_receive_error)} {}

    WorkerSocket(socket::Socket<Protocol> socket, shared::ReadCallback on_read,
                 shared::ErrorCallback on_send_error, shared::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket, std::move(on_send_error)},
          m_receiver{m_socket, std::move(on_read), std::move(on_receive_error)} {
        build();
    }

    ~WorkerSocket() { close(); }

    WorkerSocket(const WorkerSocket &) = delete;
    WorkerSocket &operator=(const WorkerSocket &) = delete;
    WorkerSocket(WorkerSocket &&other) = delete;
    WorkerSocket &operator=(WorkerSocket &&other) = delete;

    void add_on_read(shared::ReadCallback on_read) & { m_receiver.add_on_read(std::move(on_read)); }

    void add_on_send_error(shared::ErrorCallback on_send_error) & {
        m_sender.add_on_error(std::move(on_send_error));
    }

    void add_on_receive_error(shared::ErrorCallback on_receive_error) & {
        m_receiver.add_on_error(std::move(on_receive_error));
    }

    void build() & {
        m_sender.build();
        m_receiver.build();
    }


    template <shared::HandlerController Controller>
    void start(Controller &controller) {
        m_sender.template create<Controller>(controller);
        m_receiver.template create<Controller>(controller);
    }

    void close() {
        core::logger::debug("io/worker", "fd {} closed", get_fd());
        m_sender.set_closed();
        m_receiver.set_closed();
        m_socket.sync_close();
    }

    const Sender<socket::Socket<Protocol>, socket::SocketStatus> &get_sender() const noexcept {
        return m_sender;
    }
    Sender<socket::Socket<Protocol>, socket::SocketStatus> &get_sender() { return m_sender; }
    const Receiver<socket::Socket<Protocol>, socket::SocketStatus> &get_receiver() const noexcept {
        return m_receiver;
    }
    Receiver<socket::Socket<Protocol>, socket::SocketStatus> &get_receiver() noexcept {
        return m_receiver;
    }
    [[nodiscard]] const socket::SOCKET &get_fd() const noexcept { return m_socket.get_fd(); }
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

    ServerFlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller)
        : m_base_socket{std::move(end), leverager}, m_leverager{leverager},
          m_controller{controller}, m_workers{}, m_on_established{nullptr} {};

    ~ServerFlowSocket() {
        m_base_socket.set_closed();
        core::logger::debug("io/flow", "closing base socket {}",
                            m_base_socket.get_endpoint().to_string());
        for (auto &[fd, worker] : m_workers) {
            worker->close();
        }
    }

    ServerFlowSocket(const ServerFlowSocket &) = delete;
    ServerFlowSocket &operator=(const ServerFlowSocket &) = delete;
    ServerFlowSocket(ServerFlowSocket &&other)
        : m_base_socket{std::move(other.m_base_socket)}, m_leverager{std::move(other.m_leverager)},
          m_controller{std::move(other.m_controller)}, m_workers{std::move(other.m_workers)},
          m_on_established(std::move(other.m_on_established)) {}
    ServerFlowSocket &operator=(ServerFlowSocket &&other) {
        if (this != &other) {
            m_base_socket = std::move(other.m_base_socket);
            m_leverager = std::move(other.m_leverager);
            m_controller = std::move(other.m_controller);
            m_workers = std::move(other.m_workers);
            m_on_established = std::move(other.m_on_established);
        }
        return *this;
    };


    void add_on_accept(ConnectionEstablishedCallback established) & {
        m_on_established = std::move(established);
    }

    void build() & {
        if (!m_on_established) {
            throw std::runtime_error(
                "ConnectionEstablished callback must be set before building the ServerFlowSocket");
        }
        helper();
        start();
    }

    void start() {
        core::logger::debug("io/flow", "base socket {} start",
                            m_base_socket.get_endpoint().to_string());
        m_base_socket.template create<Controller>(m_controller);
    }

    shared::SendCallback on_send(socket::SOCKET fd) {
        auto value = m_workers.find(fd);
        if (value) {
            return [sender = &value->get_sender()](utils::buffering::BufferNode &&node) {
                sender->send(std::move(node));
            };
        }
        return nullptr;
    }

  private:
    void helper() {
        m_base_socket.add_on_accept([this](socket::Socket<Protocol> accepted_socket) mutable {
            core::logger::debug("io/flow", "accepted {} from {}",
                                m_base_socket.get_endpoint().to_string(),
                                accepted_socket.get_endpoint().to_string());
            auto connector = std::make_unique<ConnectorSocket<Protocol>>(
                std::move(accepted_socket),
                [this](socket::Socket<Protocol> encrypted_socket) mutable {
                    auto worker = std::make_shared<WorkerSocket<Protocol>>(
                        std::move(encrypted_socket),
                        [this](socket::SOCKET fd, int err) {
                            m_workers.erase(fd);
                            core::logger::error("io/flow/server", "send error on socket {}: {}", fd,
                                                err);
                        },
                        [this](socket::SOCKET fd, int err) {
                            m_workers.erase(fd);
                            core::logger::error("io/flow/server", "receive error on socket {}: {}",
                                                fd, err);
                        });

                    auto read_calback = m_on_established(
                        [worker](utils::buffering::BufferNode &&node) {
                            worker->get_sender().send(std::move(node));
                        },
                        [this, worker]() {
                            core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                            m_workers.erase(worker->get_fd());
                        });

                    worker->add_on_read(std::move(read_calback));
                    worker->build();

                    worker->template start<Controller>(m_controller);

                    core::logger::debug("io/flow", "fd {} connected under {}", worker->get_fd(),
                                        m_base_socket.get_endpoint().to_string());

                    m_workers.insert(worker->get_fd(), std::move(worker));
                });

            core::logger::debug("io/flow", "fd {} handshake start", accepted_socket.get_fd());
            connector->template create<Controller>(m_controller);
            connector->what_this_is_me(std::move(connector));
        });

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

    ClientFlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller)
        : m_endpoint{std::move(end)}, m_leverager{leverager}, m_controller{controller}, m_worker{},
          m_on_established{nullptr} {}

    ~ClientFlowSocket() {
        for (auto &[fd, worker] : m_worker) {
            worker->close();
        }
    }

    ClientFlowSocket(const ClientFlowSocket &) = delete;
    ClientFlowSocket &operator=(const ClientFlowSocket &) = delete;
    ClientFlowSocket(ClientFlowSocket &&other)
        : m_endpoint{std::move(other.m_endpoint)}, m_leverager{std::move(other.m_leverager)},
          m_controller{std::move(other.m_controller)}, m_worker{std::move(other.m_worker)},
          m_on_established{std::move(other.m_on_established)} {}
    ClientFlowSocket &operator=(ClientFlowSocket &&other) {
        if (this != &other) {
            m_endpoint = std::move(other.m_endpoint);
            m_leverager = std::move(other.m_leverager);
            m_controller = std::move(other.m_controller);
            m_worker = std::move(other.m_worker);
            m_on_established = std::move(other.m_on_established);
        }
        return *this;
    }

    void add_on_accept(ConnectionEstablishedCallback established) & {
        m_on_established = std::move(established);
    }

    void build() & {
        if (!m_on_established) {
            throw std::runtime_error(
                "ConnectionEstablished callback must be set before building the ClientFlowSocket");
        }
        helper();
    }

    shared::SendCallback on_send() {
        if (auto value = m_worker.value(); m_worker.has_value()) {
            return [sender = &value->get_sender()](utils::buffering::BufferNode &&node) {
                sender->send(std::move(node));
            };
        }
        return nullptr;
    }

  private:
    static socket::Socket<Protocol> create_socket(socket::Endpoint endpoint) {
        socket::Socket<Protocol> socket{std::move(endpoint)};
        socket.add_alpn_proto("h2");

        socket.generate_certificate("./server.crt", "server.key");
        socket.load_certificate("./server.crt", "server.key");

        auto connect_status = socket.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            core::logger::error("io/flow/client", "connect to {} failed",
                                socket.get_endpoint().to_string());
            return;
        }
        socket.set_non_blocking();
        return socket;
    }

    void helper() {
        core::logger::debug("io/flow", "base socket {} start", m_endpoint.to_string());

        auto socket = create_socket(m_endpoint);

        core::logger::debug("io/flow", "fd {} handshake start", socket.get_fd());

        auto connector = std::make_unique<ConnectorSocket<Protocol>>(
            std::move(socket), [this](socket::Socket<Protocol> encrypted_socket) mutable {
                auto worker = std::make_shared<WorkerSocket<Protocol>>(
                    std::move(encrypted_socket),
                    [this](socket::SOCKET fd, int err) {
                        m_worker.reset();
                        core::logger::error("io/flow/server", "send error on socket {}: {}", fd,
                                            err);
                    },
                    [this](socket::SOCKET fd, int err) {
                        m_worker.reset();
                        core::logger::error("io/flow/server", "receive error on socket {}: {}", fd,
                                            err);
                    });

                auto read_calback = m_on_established(
                    [worker](utils::buffering::BufferNode &&node) {
                        worker->get_sender().send(std::move(node));
                    },
                    [this, worker]() {
                        core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                        m_worker.reset();
                    });

                worker->add_on_read(std::move(read_calback));
                worker->build();

                worker->template start<Controller>(m_controller);

                core::logger::debug("io/flow", "fd {} connected under {}", worker->get_fd(),
                                    m_endpoint.to_string());

                m_worker.emplace(std::move(worker));
            });


        connector->template create<Controller>(m_controller);
        connector->what_this_is_me(std::move(connector));
    }

    socket::Endpoint m_endpoint;
    std::reference_wrapper<Leverager> m_leverager;
    std::reference_wrapper<Controller> m_controller;
    std::optional<std::shared_ptr<WorkerSocket<Protocol>>> m_worker;
    ConnectionEstablishedCallback m_on_established;
};

} // namespace io::base::flow::sync
