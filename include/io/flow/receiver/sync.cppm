export module io_flow_receiver:sync;

import std;
import core_logger;
import shared;
import interfaces;
import io_base_socket;
import utils_buffering;

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncReceivable<Worker, Status, std::byte *, Args...>
class Receiver : public shared::HandlerBase {
  public:
    Receiver(Worker &worker)
        : m_worker{worker}, m_pool{}, m_on_read{nullptr}, m_on_error{nullptr}, m_stalled{false}, m_closed{true} {
    }

    Receiver(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_read{nullptr}, m_on_error{std::move(on_error)}, m_stalled{false},
          m_closed{false} {
    }

    Receiver(Worker &worker, shared::ReadCallback on_read, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)}, m_stalled{false},
          m_closed{false} {
        build();
    }

    ~Receiver() = default;

    Receiver(const Receiver &) = delete;
    Receiver &operator=(const Receiver &) = delete;
    Receiver(Receiver &&other) = delete;
    Receiver &operator=(Receiver &&other) = delete;

    void add_on_read(shared::ReadCallback on_read) { m_on_read = std::move(on_read); }

    void add_on_error(shared::ErrorCallback on_error) { m_on_error = std::move(on_error); }

    void build() {
        if (!m_on_read) {
            throw std::runtime_error("Read callback must be set before building the Receiver");
        }
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Receiver");
        }
    }

    std::string_view get_name() const noexcept override { return "Receiver - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            if (resume()) {
                core::logger::debug("io/recv", "fd {} rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("io/recv", "fd {} sys error: {} ({})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/recv", "fd {} exception: {}", m_worker.get().get_fd(), e.what());
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("io/recv", "fd {} unknown exception", m_worker.get().get_fd());
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    bool resume() {
        if (m_closed) {
            return false;
        }
        if (!m_stalled) {
            m_stalled = true;
            arm_read();
        }
        return true;
    }

    void set_closed() noexcept {
        m_closed = true;
    }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }

  private:
    void arm_read() {
        const auto fd = m_worker.get().get_fd();
        if (m_closed) {
            core::logger::warning("io/recv", "fd {} read on closed", fd);
            m_stalled = false;
            return;
        }

        auto slot = m_pool.acquire();

        auto [result, status] =
            m_worker.get().sync_receive(slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0);

        switch (status.get_status()) {
        case socket::VALUES::VALID: {
            const auto bytes = static_cast<std::size_t>(result);

            core::logger::debug("io/recv", "fd {} rx {} bytes", fd, bytes);

            m_pool.notify_read(slot, bytes);
            m_on_read(m_pool.get_view());
            m_stalled = false;
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED:
            core::logger::debug("io/recv", "fd {} would block", fd);
            m_stalled = false;
            return;
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT: {
            core::logger::warning("io/recv", "fd {} read error: {}", fd, result);
            m_closed = true;
            m_on_error(fd, status.get_value());
            return;
        }
        }
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ReadCallback m_on_read;
    shared::ErrorCallback m_on_error;
    bool m_stalled;
    bool m_closed;
};

static_assert(interfaces::io::SyncReceivable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus, std::byte *>);

} // namespace io::base::flow::sync
