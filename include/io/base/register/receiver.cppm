export module io_base_register:receiver;

import std;
import shared;
import io_base_socket;
import io_base_buffering;
import io_base_leverage;

export namespace io::base::connect {

class Receiver : public shared::HandlerBase {
  public:
    using LeveragerType = leverage::Leverager<leverage::Context>;
    using ReadCallback = std::move_only_function<void(buffering::BufferView)>;
    using ErrorCallback = std::move_only_function<void(socket::SOCKET, int)>;

    Receiver(int fd, LeveragerType &leverager, ReadCallback on_read, ErrorCallback on_error)
        : m_leverager{leverager}, m_pool{}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)},
          m_threshold{0}, m_fd{fd}, m_stalled{false}, m_closed{true} {
        register_file();
    }

    Receiver(const Receiver &) = delete;
    Receiver &operator=(const Receiver &) = delete;
    Receiver(Receiver &&) = delete;
    Receiver &operator=(Receiver &&) = delete;

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
        if (!m_stalled && m_threshold > 0) {
            m_stalled = true;
            arm_read();
        }
        return true;
    }

    void register_file() { m_leverager.get().register_file(m_fd); }
    void unregister_file() { m_leverager.get().unregister_file(m_fd); }

    void set_threshold(std::size_t threshold) noexcept { m_threshold = threshold; }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }
    [[nodiscard]] int get_fd() const noexcept { return m_fd; }

  private:
    void arm_read() {
        auto slot = m_pool.acquire();

        if (auto opt_slot = slot.value(); slot.has_value()) {
            m_leverager.get().async_read(m_fd, opt_slot->get_data(), static_cast<unsigned>(opt_slot->get_size()), 0,
                                         [this](int result) mutable { on_read_complete(result); });
        } else {
            m_stalled = false;
        }
    }

    void on_read_complete(int result) {
        if (m_closed)
            return;

        if (result <= 0) {
            m_closed = true;
            m_on_error(m_fd, -result);
            return;
        }

        const auto bytes = static_cast<std::size_t>(result);

        if (bytes < m_threshold) {
            m_threshold -= bytes;
        } else {
            m_threshold = 0;
            m_on_read(m_pool.get_view());
        }

        m_pool.notify_read(bytes);
        m_stalled = false;
    }

    std::reference_wrapper<LeveragerType> m_leverager;
    buffering::BufferPool m_pool;
    ReadCallback m_on_read;
    ErrorCallback m_on_error;
    std::size_t m_threshold;
    int m_fd;
    bool m_stalled;
    bool m_closed;
};

} // namespace io::base::connect
