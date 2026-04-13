export module io_base_register:socket;

import std;
import io_base_socket;
import io_base_buffering;
import io_base_leverage;
import shared;
import :sender;
import :receiver;

export namespace transport::base::connect {

template <socket::Protocol protocol>
class SocketWrapper {
  public:
    SocketWrapper(socket::Socket<protocol> socket, leverage::Leverager<leverage::Context> &leverager,
                  Receiver::ReadCallback on_read, Sender::ErrorCallback on_send_error,
                  Receiver::ErrorCallback on_receive_error)
        : m_socket{std::move(socket)}, m_sender{m_socket.get_native_handle(), leverager, std::move(on_send_error)},
          m_receiver{m_socket.get_native_handle(), leverager, std::move(on_read), std::move(on_receive_error)} {}

    SocketWrapper(const SocketWrapper &) = delete;
    SocketWrapper &operator=(const SocketWrapper &) = delete;
    SocketWrapper(SocketWrapper &&) = delete;
    SocketWrapper &operator=(SocketWrapper &&) = delete;

    ~SocketWrapper() = default;

    const Sender &get_sender() const noexcept { return m_sender; }
    const Receiver &get_receiver() const noexcept { return m_receiver; }

  private:
    socket::Socket<protocol> m_socket;
    Sender m_sender;
    Receiver m_receiver;
};

} // namespace transport::base::connect
