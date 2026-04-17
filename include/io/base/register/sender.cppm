export module io_base_register:sender;

import std;
import io_base_buffering;
import io_base_leverage;
import io_base_socket;
import shared;

export namespace io::base::connect {

class Sender : public shared::HandlerBase {
  public:
    using LeveragerType = leverage::Leverager<leverage::Context>;
    using ErrorCallback = std::move_only_function<void(socket::SOCKET, int)>;

    Sender(int fd, LeveragerType &leverager, ErrorCallback on_error)
        : m_leverager{leverager}, m_pool{}, m_on_error{std::move(on_error)}, m_fd{fd}, m_stalled{false},
          m_closed{true} {
        register_file();
    }

    Sender(const Sender &) = delete;
    Sender &operator=(const Sender &) = delete;
    Sender(Sender &&) = delete;
    Sender &operator=(Sender &&) = delete;

    void send(buffering::BufferNode &slot) { m_pool.push(std::move(slot)); }

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
        return [this]() noexcept { m_closed = true; };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                m_on_error(m_fd, e.code().value());
            } catch (...) {
                m_on_error(m_fd, -1);
            }
        };
    }

    bool resume() {
        if (m_closed) {
            return false;
        }
        if (m_stalled) {
            m_stalled = true;
            arm_write();
        }
        return true;
    }

    void register_file() { m_leverager.get().register_file(m_fd); }
    void unregister_file() { m_leverager.get().unregister_file(m_fd); }

    void set_closed() noexcept { m_closed = true; }

    bool get_stalled() const noexcept { return m_stalled; }
    bool get_closed() const noexcept { return m_closed; }
    int get_fd() const noexcept { return m_fd; }

  private:
    void arm_write() {
        auto view = m_pool.get_view();

        auto slot = view.next();
        if (!slot) {
            m_stalled = false;
            return;
        }

        m_leverager.get().async_write(
            m_fd, slot->get_data(), static_cast<unsigned>(slot->get_size()), 0,
            [this, s = std::move(*slot)](int result) mutable { on_write_complete(std::move(s), result); });
    }

    void on_write_complete(buffering::BufferNode slot, int result) {
        if (m_closed)
            return;

        if (result <= 0) {
            m_closed = true;
            m_on_error(m_fd, -result);
            return;
        }

        slot.~BufferNode();

        m_stalled = false;
    }

    std::reference_wrapper<LeveragerType> m_leverager;
    buffering::BufferPool m_pool;
    ErrorCallback m_on_error;
    int m_fd;
    bool m_stalled;
    bool m_closed;
};

} // namespace io::base::connect
