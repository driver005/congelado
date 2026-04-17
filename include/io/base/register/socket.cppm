export module io_base_register:socket;

import std;
import io_base_socket;
import io_base_buffering;
import io_base_leverage;
import shared;
import hashmap;
import :sender;
import :receiver;

export namespace io::base::connect {

template <socket::Protocol protocol>
class SocketBase : shared::HandlerBase {
  public:
    using OnAcceptCallback = std::move_only_function<void(socket::Socket<protocol>)>;

    SocketBase(socket::Endpoint end, OnAcceptCallback on_accept)
        : m_socket{socket::Socket<protocol>{end}}, m_on_accept{on_accept}, m_closed{false} {
        start();
    };

    void start() {
        m_socket.bind();
        m_socket.listen();
    }

    void close() { m_closed = true; }

    void accept() {
        socket::Socket<protocol> accepted_socket = m_socket.accept();
        if (accepted_socket) {
            m_on_accept(std::move(accepted_socket));
        }
    }

    bool resume() {
        if (m_closed) {
            return false;
        }
        accept();
        return true;
    }

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
        return [this]() noexcept { m_socket.close(); };
    }

  private:
    socket::Socket<protocol> m_socket;
    OnAcceptCallback m_on_accept;
    bool m_closed;
};

template <socket::Protocol protocol>
class SocketConnector : shared::HandlerBase {
  public:
    using OnHandshakeComplete = std::function<void()>;

    enum class ConnectResult { Success, Error, InProgress };


    // Controll if should reshedule
    ConnectResult handshake() {
        socket::SocketStatus status = m_socket.handshake();
        if (status.is_valid()) {
            m_on_success();
            return ConnectResult::Success;
        } else if (status.is_errored()) {
            return ConnectResult::Error;
        }

        return ConnectResult::InProgress;
    }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            auto result = handshake();
            if (result == ConnectResult::InProgress) {
                shared::this_handler::shedule();
            } else if (result == ConnectResult::Error) {
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_socket.close(); };
    }

  private:
    std::reference_wrapper<socket::Socket<protocol>> m_socket;
    OnHandshakeComplete m_on_success;
};

template <socket::Protocol protocol>
class SocketWorker : shared::HandlerBase {
  public:
    SocketWorker(socket::Socket<protocol> socket, leverage::Leverager<leverage::Context> &leverager,
                 Receiver::ReadCallback on_read, Sender::ErrorCallback on_send_error,
                 Receiver::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket.get_native_handle(), leverager, std::move(on_send_error)},
          m_receiver{m_socket.get_native_handle(), leverager, std::move(on_read), std::move(on_receive_error)} {}

    SocketWorker(const SocketWorker &) = delete;
    SocketWorker &operator=(const SocketWorker &) = delete;
    SocketWorker(SocketWorker &&) = delete;
    SocketWorker &operator=(SocketWorker &&) = delete;

    ~SocketWorker() { close(); };

    void close() {
        m_sender.unregister_file();
        m_receiver.unregister_file();
        m_socket.close();
    }

    const Sender &get_sender() const noexcept { return m_sender; }
    const Receiver &get_receiver() const noexcept { return m_receiver; }
    socket::SOCKET get_fd() const noexcept { return m_socket.get_native_handle(); }

  private:
    socket::Socket<protocol> m_socket;
    Sender m_sender;
    Receiver m_receiver;
};

// Wrapper that connects the base cocket to the worker and manages the types for the thread model.
template <socket::Protocol protocol, shared::HandlerController Controller>
class SocketWrapper {
  public:
    SocketWrapper(socket::Endpoint end, Controller controller, leverage::Leverager<leverage::Context> &leverager,
                  Receiver::ReadCallback on_worker_read)
        : m_base_socket{init_base_socket(std::move(end), leverager, std::move(on_worker_read))},
          m_controller{controller}, m_workers{} {}

    ~SocketWrapper() {
        m_base_socket.close();
        for (auto &[fd, worker] : m_workers) {
            worker.close();
        }
    }

  private:
    inline SocketBase<protocol> init_base_socket(socket::Endpoint end,
                                                 leverage::Leverager<leverage::Context> &leverager,
                                                 Receiver::ReadCallback on_worker_read) {
        return SocketBase<protocol>{
            std::move(end),
            [this, &leverager, &on_worker_read](socket::Socket<protocol> accepted_socket) {
                SocketConnector<protocol> connector{
                    accepted_socket,
                    [this, &leverager, &on_worker_read, &accepted_socket]() {
                        auto worker = SocketWorker<protocol>{
                            std::move(accepted_socket),
                            leverager,
                            std::move(on_worker_read),
                            [this](socket::SOCKET fd, int err) {
                                m_workers.erase(fd);
                                std::println("Error while sending data on socket {}: {}", fd, err);
                            },
                            [&](socket::SOCKET fd, int err) {
                                m_workers.erase(fd);
                                std::println("Error while receiving data on socket {}: {}", fd, err);
                            },
                        };
                        worker.template plug_into<Controller>(m_controller);
                        m_workers.insert(worker.get_fd(), std::move(worker));
                    },
                };

                connector.template plug_into<Controller>(m_controller);
            },
        };
    }

    SocketBase<protocol> m_base_socket;
    Controller m_controller;
    hashmap::swiss::SwissHashMap<socket::SOCKET, SocketWorker<protocol>> m_workers;
};

} // namespace io::base::connect
