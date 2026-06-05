export module io_flow_sender:sync;

import std;
import core_logger;
import interfaces;
import utils_buffering;
import io_base_leverage;
import io_base_socket;
import shared;

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncSendable<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    Sender(Worker &worker) : m_worker{worker}, m_pool{}, m_on_error{nullptr}, m_stalled{false}, m_closed{false} {
    }

    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_error{std::move(on_error)}, m_stalled{false}, m_closed{false} {
        build();
    }

    ~Sender() = default;

    Sender(const Sender &) = delete;
    Sender &operator=(const Sender &) = delete;
    Sender(Sender &&) = delete;
    Sender &operator=(Sender &&) = delete;

    void add_on_error(shared::ErrorCallback on_error) & { m_on_error = std::move(on_error); }

    void build() {
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Sender");
        }
    }

    void send(utils::buffering::BufferNode slot) {
        core::logger::debug("io/send", "fd {} enqueue {} bytes", m_worker.get().get_fd(), slot.get_written());
        m_pool.push(std::move(slot));
    }

    std::string_view get_name() const noexcept override { return "Sender - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            if (resume()) {
                core::logger::debug("io/send", "fd {} rescheduled", m_worker.get().get_fd());
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
                core::logger::warning("io/send", "fd {} sys error: {} ({})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/send", "fd {} exception: {}", m_worker.get().get_fd(), e.what());
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("io/send", "fd {} unknown exception", m_worker.get().get_fd());
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    shared::SendCallback get_submitter() {
        return [this](utils::buffering::BufferNode &&node) { this->send(std::move(node)); };
    }

    bool resume() {
        if (m_closed) {
            return false;
        }
        if (!m_stalled) {
            m_stalled = true;
            arm_write();
        }
        return true;
    }

    void set_closed() noexcept {
        m_closed = true;
    }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }

    [[nodiscard]] bool has_on_error() const noexcept { return !m_on_error; }

  private:
    void arm_write() {
        const auto fd = m_worker.get().get_fd();
        if (m_closed) {
            core::logger::warning("io/send", "fd {} write on closed", fd);
            m_stalled = false;
            return;
        }

        auto &view = m_pool.get_view();
        auto [data, size] = view.front();

        if ((data == nullptr) || size == 0) {
            m_stalled = false;
            return;
        }

        core::logger::debug("io/send", "fd {} tx attempt {} bytes", fd, size);

        auto [result, status] = m_worker.get().sync_send(data, size);

        switch (status.get_status()) {
        case socket::VALUES::VALID: {
            core::logger::debug("io/send", "fd {} tx {} bytes", fd, result);
            view.consume(result);
            m_stalled = false;
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED: {
            core::logger::debug("io/send", "fd {} would block, reschedule", fd);
            m_stalled = false;
            return;
        }
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT:
            core::logger::warning("io/send", "fd {} send error: {}", fd, result);
            m_closed = true;
            m_on_error(fd, status.get_value());
            return;
        }
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ErrorCallback m_on_error;
    bool m_stalled;
    bool m_closed;
};


static_assert(interfaces::io::SyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::sync
