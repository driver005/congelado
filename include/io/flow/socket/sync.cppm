export module io_flow_socket:sync;

import std;
import core_logger;
import io_base_socket;
import io_base_buffering;
import io_base_leverage;
import io_flow_sender;
import io_flow_receiver;
import interfaces;
import shared;
import hashmap;

export namespace io::base::flow::sync {

using Leverager = leverage::Leverager<leverage::Context>;

template <socket::Protocol protocol>
class BaseSocket : public shared::HandlerBase {
  public:
    using OnAcceptCallback = std::move_only_function<void(socket::Socket<protocol>)>;

    BaseSocket(socket::Endpoint end, Leverager &leverager)
        : m_socket{socket::Socket<protocol, true>{std::move(end), std::ref(leverager)}}, m_on_accept{nullptr},
          m_closed{false} {
        core::logger::debug("BaseSocket", "Created for endpoint `{}`", m_socket.get_endpoint().to_string());
    };

    BaseSocket<protocol> &&add_on_accept(OnAcceptCallback on_accept) && {
        m_on_accept = std::move(on_accept);
        return std::move(*this);
    }

    void add_on_accept(OnAcceptCallback on_accept) & { m_on_accept = std::move(on_accept); }

    BaseSocket<protocol> &&build() && {
        if (!m_on_accept) {
            throw std::runtime_error("OnAccept callback must be set before building the BaseSocket");
        }
        start();
        return std::move(*this);
    }

    void build() & {
        if (!m_on_accept) {
            throw std::runtime_error("OnAccept callback must be set before building the BaseSocket");
        }
        start();
    }

    void start() {
        m_socket.set_non_blocking();
        m_socket.bind();
        m_socket.listen();
    }

    void set_closed() { m_closed = true; }

    void accept() {
        core::logger::debug("BaseSocket", "Endpoint `{}` checking for new connections",
                            m_socket.get_endpoint().to_string());
        socket::Socket<protocol> accepted_socket = m_socket.sync_accept();
        if (accepted_socket) {
            core::logger::info("BaseSocket", "Endpoint `{}` accepted new connection from `{}`",
                               m_socket.get_endpoint().to_string(), accepted_socket.get_endpoint().to_string());
            accepted_socket.set_non_blocking();
            m_on_accept(std::move(accepted_socket));
        }

        core::logger::info("BaseSocket", "Endpoint `{}` has NO new connections", m_socket.get_endpoint().to_string());
    }

    bool resume() {
        if (m_closed) {
            core::logger::warning("BaseSocket", "Endpoint `{}` is closed, cannot resume",
                                  m_socket.get_endpoint().to_string());
            return false;
        }
        accept();
        return true;
    }

    std::string_view name() const noexcept override { return "BaseSocket - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            core::logger::debug("BaseSocket", "Endpoint `{}` on_execute is being executed",
                                m_socket.get_endpoint().to_string());
            if (resume()) {
                core::logger::info("BaseSocket", "Endpoint `{}` rescheduled", m_socket.get_endpoint().to_string());
                shared::this_handler::shedule();
            } else {
                core::logger::info("BaseSocket", "Endpoint `{}` released", m_socket.get_endpoint().to_string());
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            core::logger::debug("BaseSocket", "Endpoint `{}` on_released is being executed",
                                m_socket.get_endpoint().to_string());
            m_socket.sync_close();
        };
    }


    const socket::SOCKET &get_fd() const noexcept { return m_socket.get().get_fd(); }
    socket::SOCKET &get_fd() noexcept { return m_socket.get().get_fd(); }
    const socket::Endpoint get_endpoint() const noexcept { return m_socket.get_endpoint(); }
    socket::Endpoint get_endpoint() noexcept { return m_socket.get_endpoint(); }
    socket::Endpoint get_recived_endpoint() noexcept { return m_socket.get_recived_endpoint(); }

  private:
    socket::Socket<protocol, true> m_socket;
    OnAcceptCallback m_on_accept;
    bool m_closed;
};

template <socket::Protocol protocol>
class ConnectorSocket : public shared::HandlerBase {
  public:
    using OnHandshakeComplete = std::function<void(socket::Socket<protocol>)>;

    ConnectorSocket(socket::Socket<protocol> socket, OnHandshakeComplete on_success)
        : m_socket{std::move(socket)}, m_on_success{std::move(on_success)} {
        core::logger::debug("ConnectorSocket", "Created for socket `{}`", get_fd());
    };

    ~ConnectorSocket() { core::logger::debug("ConnectorSocket", "Destroyed for socket `{}`", get_fd()); }

    enum class ConnectResult { Success, Error, InProgress };

    // Controll if should reshedule
    ConnectResult handshake() {
        socket::SocketStatus status = m_socket.sync_handshake();
        if (status.is_valid()) {
            core::logger::info("ConnectorSocket", "Socket `{}` handshake completed successfully", get_fd());
            m_on_success(std::move(m_socket));
            return ConnectResult::Success;
        } else if (status.is_errored()) {
            core::logger::warning("ConnectorSocket", "Socket `{}` handshake failed", get_fd());
            return ConnectResult::Error;
        }

        core::logger::info("ConnectorSocket", "Socket `{}` handshake in progress", get_fd());
        return ConnectResult::InProgress;
    }

    std::string_view name() const noexcept override { return "ConnectorSocket - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            core::logger::debug("ConnectorSocket", "Socket `{}` on_execute is being executed", get_fd());
            auto result = handshake();
            if (result == ConnectResult::InProgress) {
                core::logger::info("ConnectorSocket", "Socket `{}` rescheduled", get_fd());
                shared::this_handler::shedule();
            } else if (result == ConnectResult::Error) {
                core::logger::info("ConnectorSocket", "Socket `{}` released", get_fd());
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            core::logger::debug("ConnectorSocket", "Socket `{}` on_released is being executed", get_fd());
            m_socket.sync_close();
        };
    }

    const socket::SOCKET &get_fd() const noexcept { return m_socket.get_fd(); }
    socket::SOCKET &get_fd() noexcept { return m_socket.get_fd(); }

  private:
    socket::Socket<protocol> m_socket;
    OnHandshakeComplete m_on_success;
};

template <socket::Protocol protocol>
class WorkerSocket {
  public:
    WorkerSocket(socket::Socket<protocol> socket)
        : m_socket{std::move(socket)}, m_sender{m_socket}, m_receiver{m_socket} {
        core::logger::debug("WorkerSocket", "Created for socket `{}`", get_fd());
    }

    WorkerSocket<protocol> &&add_on_read(shared::ReadCallback on_read) && {
        m_receiver = std::move(m_receiver).add_on_read(std::move(on_read));
        return std::move(*this);
    }

    void add_on_read(shared::ReadCallback on_read) & { m_receiver.add_on_read(std::move(on_read)); }

    WorkerSocket<protocol> &&add_on_send_error(shared::ErrorCallback on_send_error) && {
        m_sender = std::move(m_sender).add_on_error(std::move(on_send_error));
        return std::move(*this);
    }

    void add_on_send_error(shared::ErrorCallback on_send_error) & { m_sender.add_on_error(std::move(on_send_error)); }

    WorkerSocket<protocol> &&add_on_receive_error(shared::ErrorCallback on_receive_error) && {
        m_receiver = std::move(m_receiver).add_on_error(std::move(on_receive_error));
        return std::move(*this);
    }

    void add_on_receive_error(shared::ErrorCallback on_receive_error) & {
        m_receiver.add_on_error(std::move(on_receive_error));
    }

    WorkerSocket<protocol> &&build() && {
        m_sender = std::move(m_sender).build();
        m_receiver = std::move(m_receiver).build();
        return std::move(*this);
    }

    void build() & {
        m_sender.build();
        m_receiver.build();
    }

    WorkerSocket(socket::Socket<protocol> socket, shared::ReadCallback on_read, shared::ErrorCallback on_send_error,
                 shared::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket, std::move(on_send_error)},
          m_receiver{m_socket, std::move(on_read), std::move(on_receive_error)} {
        core::logger::debug("WorkerSocket", "Created for socket `{}`", get_fd());
    }

    ~WorkerSocket() {
        core::logger::debug("WorkerSocket", "Destructor called for socket `{}`", get_fd());
        close();
    }

    WorkerSocket(const WorkerSocket &) = delete;
    WorkerSocket &operator=(const WorkerSocket &) = delete;
    WorkerSocket(WorkerSocket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_sender{std::move(other.m_sender)},
          m_receiver{std::move(other.m_receiver)} {}
    WorkerSocket &operator=(WorkerSocket &&other) {
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_sender = std::move(other.m_sender);
            m_receiver = std::move(other.m_receiver);
        }
        return *this;
    };

    template <shared::HandlerController Controller>
    void start(Controller &controller) {
        core::logger::debug("WorkerSocket", "Starting handler for socket `{}`", get_fd());
        m_sender.template create<Controller>(controller);
        m_receiver.template create<Controller>(controller);
    }

    void close() {
        core::logger::info("WorkerSocket", "Closing socket `{}`", get_fd());
        m_sender.set_closed();
        m_receiver.set_closed();
        m_socket.sync_close();
    }

    const Sender<socket::Socket<protocol>, socket::SocketStatus> &get_sender() const noexcept { return m_sender; }
    Sender<socket::Socket<protocol>, socket::SocketStatus> &get_sender() { return m_sender; }
    const Receiver<socket::Socket<protocol>, socket::SocketStatus> &get_receiver() const noexcept { return m_receiver; }
    Receiver<socket::Socket<protocol>, socket::SocketStatus> &get_receiver() noexcept { return m_receiver; }
    const socket::SOCKET &get_fd() const noexcept { return m_socket.get_fd(); }
    socket::SOCKET &get_fd() noexcept { return m_socket.get_fd(); }

  private:
    socket::Socket<protocol> m_socket;
    Sender<socket::Socket<protocol>, socket::SocketStatus> m_sender;
    Receiver<socket::Socket<protocol>, socket::SocketStatus> m_receiver;
};

// Wrapper that connects the base cocket to the worker and manages the types for the thread model.
template <shared::HandlerController Controller, socket::Protocol protocol>
class FlowSocket {
  public:
    using ConnectionEstablishedCallback =
        std::move_only_function<shared::ReadCallback(shared::SendCallback, shared::CloseCallback)>;

    FlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller)
        : m_base_socket{std::move(end), leverager}, m_leverager{leverager}, m_controller{controller}, m_workers{} {
        core::logger::debug("FlowSocket", "Created for endpoint `{}`", m_base_socket.get_endpoint().to_string());
    };

    FlowSocket<Controller, protocol> &&add_on_accept(ConnectionEstablishedCallback established) && {
        helper(std::move(established));
        return std::move(*this);
    }

    void add_on_accept(ConnectionEstablishedCallback established) & { helper(std::move(established)); }

    FlowSocket<Controller, protocol> &&build() && {
        start();
        return std::move(*this);
    }

    void build() & { start(); }

    ~FlowSocket() {
        core::logger::debug("FlowSocket", "Destructor called for endpoint `{}`",
                            m_base_socket.get_endpoint().to_string());
        m_base_socket.set_closed();
        core::logger::info("FlowSocket", "Endpoint `{}` closeing base socket",
                           m_base_socket.get_endpoint().to_string());
        for (auto &[fd, worker] : m_workers) {
            core::logger::info("FlowSocket", "Endpoint `{}` closing worker socket `{}`",
                               m_base_socket.get_endpoint().to_string(), fd);
            worker->close();
        }
    }

    FlowSocket(const FlowSocket &) = delete;
    FlowSocket &operator=(const FlowSocket &) = delete;
    FlowSocket(FlowSocket &&other)
        : m_base_socket{std::move(other.m_base_socket)}, m_leverager{std::move(other.m_leverager)},
          m_controller{std::move(other.m_controller)}, m_workers{std::move(other.m_workers)} {}
    FlowSocket &operator=(FlowSocket &&other) {
        if (this != &other) {
            m_base_socket = std::move(other.m_base_socket);
            m_leverager = std::move(other.m_leverager);
            m_controller = std::move(other.m_controller);
            m_workers = std::move(other.m_workers);
        }
        return *this;
    };

    void start() {
        core::logger::debug("FlowSocket", "Starting base socket for endpoint `{}`",
                            m_base_socket.get_endpoint().to_string());
        m_base_socket.template create<Controller>(m_controller);
    }

    shared::SendCallback on_send(socket::SOCKET fd) {
        auto value = m_workers.find(fd);
        if (value) {
            return
                [sender = &value->get_sender()](base::buffering::BufferNode &&node) { sender->send(std::move(node)); };
        }
        return nullptr;
    }

  private:
    inline void helper(ConnectionEstablishedCallback established) {
        m_base_socket.add_on_accept([this, &established](socket::Socket<protocol> accepted_socket) mutable {
            core::logger::info("FlowSocket", "Endpoint `{}` accepted new connection from `{}`",
                               m_base_socket.get_endpoint().to_string(), accepted_socket.get_endpoint().to_string());
            ConnectorSocket<protocol> *connector = nullptr;
            connector = new ConnectorSocket<protocol>{
                std::move(accepted_socket),
                [this, &established, connector](socket::Socket<protocol> encrypted_socket) mutable {
                    core::logger::info("FlowSocket", "Endpoint `{}` handshake completed for socket `{}`",
                                       m_base_socket.get_endpoint().to_string(), encrypted_socket.get_fd());
                    auto worker = std::make_unique<WorkerSocket<protocol>>(
                        WorkerSocket<protocol>{
                            std::move(encrypted_socket),
                        }
                            .add_on_send_error([this](socket::SOCKET fd, int err) {
                                m_workers.erase(fd);
                                std::println("Error while sending data on socket {}: {}", fd, err);
                            })
                            .add_on_receive_error([this](socket::SOCKET fd, int err) {
                                m_workers.erase(fd);
                                std::println("Error while receiving data on socket {}: {}", fd, err);
                            }));

                    auto read_calback = established(
                        [&worker](base::buffering::BufferNode &&node) {
                            core::logger::debug("WorkerSocket - SendCallback", "Socket `{}` submitting data to send ",
                                                worker->get_fd());
                            worker->get_sender().send(std::move(node));
                        },
                        [this, &worker]() {
                            core::logger::info("WorkerSocket - CloseCallback", "Socket `{}` connection closed",
                                               worker->get_fd());
                            m_workers.erase(worker->get_fd());
                        });

                    worker->add_on_read(std::move(read_calback));
                    worker->build();

                    worker->template start<Controller>(m_controller);

                    m_workers.insert(worker->get_fd(), std::move(worker));
                    delete connector;
                },
            };

            core::logger::info("FlowSocket", "endpoint `{}` starting handshake for socket `{}`",
                               m_base_socket.get_endpoint().to_string(), accepted_socket.get_fd());
            connector->template create<Controller>(m_controller);
        });

        core::logger::info("FlowSocket", "endpoint `{}` starting base socket",
                           m_base_socket.get_endpoint().to_string());
        m_base_socket.build();
    }

    BaseSocket<protocol> m_base_socket;
    std::reference_wrapper<Leverager> m_leverager;
    std::reference_wrapper<Controller> m_controller;
    hashmap::swiss::SwissHashMap<socket::SOCKET, std::unique_ptr<WorkerSocket<protocol>>> m_workers;
};

} // namespace io::base::flow::sync
