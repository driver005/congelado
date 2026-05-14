export module io_flow_receiver:sync;

import std;
import core_logger;
import shared;
import interfaces;
import io_base_socket;
import io_base_buffering;

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncReceivable<Worker, Status, std::byte *, Args...>
class Receiver : public shared::HandlerBase {
  public:
    Receiver(Worker &worker)
        : m_worker{worker}, m_pool{}, m_on_read{nullptr}, m_on_error{nullptr}, m_stalled{false}, m_closed{true} {
        core::logger::debug("Receiver", "Created for worker with FD `{}`", m_worker.get().get_fd());
    }

    Receiver(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_read{nullptr}, m_on_error{std::move(on_error)}, m_stalled{false},
          m_closed{false} {
        core::logger::debug("Receiver", "Created for worker with FD `{}`", m_worker.get().get_fd());
    }

    Receiver(Worker &worker, shared::ReadCallback on_read, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)}, m_stalled{false},
          m_closed{false} {
        core::logger::debug("Receiver", "Created for worker with FD `{}`", m_worker.get().get_fd());
        build();
    }

    ~Receiver() {
        core::logger::debug("Receiver", "Destructor called for worker with FD `{}`", m_worker.get().get_fd());
    }

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

    std::string_view name() const noexcept override { return "Receiver - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            core::logger::debug("Receiver", "FD `{}` on_execute is being executed", m_worker.get().get_fd());
            if (resume()) {
                core::logger::info("Receiver", "FD `{}` rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            core::logger::debug("Receiver", "FD `{}` on_released is being executed", m_worker.get().get_fd());
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            core::logger::debug("Receiver", "FD `{}` on_error is being executed", m_worker.get().get_fd());
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("Receiver", "FD `{}` system_error: {} (code: {})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("Receiver", "FD `{}` exception: {}", m_worker.get().get_fd(), e.what());
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("Receiver", "FD `{}` unknown exception", m_worker.get().get_fd());
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    bool resume() {
        core::logger::debug("Receiver", "FD `{}` is being executed in resume with stalled = `{}` and closed = `{}`",
                            m_worker.get().get_fd(), m_stalled, m_closed);
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
        core::logger::debug("Receiver", "FD `{}` is being marked as closed and so is the Receiver too",
                            m_worker.get().get_fd());
        m_closed = true;
    }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }

  private:
    void arm_read() {
        if (m_closed) {
            core::logger::warning("Receiver", "Attempted to read on closed socket `{}`", m_worker.get().get_fd());
            m_stalled = false;
            return;
        }

        auto slot = m_pool.acquire();

        auto [result, status] =
            m_worker.get().sync_receive(slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0);

        switch (status.get_status()) {
        case socket::VALUES::VALID: {
            const auto bytes = static_cast<std::size_t>(result);

            core::logger::info("Receiver", "FD `{}` read {} bytes", m_worker.get().get_fd(), bytes);

            m_pool.notify_read(slot, bytes);
            m_on_read(m_pool.get_view());
            m_stalled = false;
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED:
            core::logger::warning("Receiver", "FD `{}` read operation would have blocked, no data available",
                                  m_worker.get().get_fd());
            m_stalled = false;
            return;
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT: {
            core::logger::warning("Receiver", "FD `{}` read operation failed with error `{}`", m_worker.get().get_fd(),
                                  result);
            m_closed = true;
            m_on_error(m_worker.get().get_fd(), status.get_value());
            return;
        }
        }
    }

    std::reference_wrapper<Worker> m_worker;
    buffering::BufferWriter m_pool;
    shared::ReadCallback m_on_read;
    shared::ErrorCallback m_on_error;
    bool m_stalled;
    bool m_closed;
};

static_assert(interfaces::io::SyncReceivable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus, std::byte *>);

} // namespace io::base::flow::sync
