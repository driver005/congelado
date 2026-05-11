export module io_flow_sender:sync;

import std;
import core_logger;
import interfaces;
import io_base_buffering;
import io_base_leverage;
import io_base_socket;
import shared;

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncSendable<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    Sender(Worker &worker) : m_worker{worker}, m_pool{}, m_on_error{nullptr}, m_stalled{false}, m_closed{false} {
        core::logger::debug("Sender", "Created for worker with FD `{}`", m_worker.get().get_fd());
    }

    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_error{std::move(on_error)}, m_stalled{false}, m_closed{false} {
        core::logger::debug("Sender", "Created for worker with FD `{}`", m_worker.get().get_fd());
        build();
    }

    ~Sender() { core::logger::debug("Sender", "Destructor called for worker with FD `{}`", m_worker.get().get_fd()); }

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

    void send(buffering::BufferNode slot) {
        core::logger::debug("Sender", "FD `{}` adding node to send pool with size `{}`", m_worker.get().get_fd(),
                            slot.get_written());
        m_pool.push(std::move(slot));
    }

    std::string_view name() const noexcept override { return "Sender - Sync"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            core::logger::debug("Sender", "FD `{}` on_execute is being executed", m_worker.get().get_fd());
            if (resume()) {
                core::logger::info("Sender", "FD `{}` rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            core::logger::debug("Sender", "FD `{}` on_released is being executed", m_worker.get().get_fd());
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            core::logger::debug("Sender", "FD `{}` on_error is being executed", m_worker.get().get_fd());
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("Sender", "FD `{}` system_error: {} (code: {})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("Sender", "FD `{}` exception: {}", m_worker.get().get_fd(), e.what());
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("Sender", "FD `{}` unknown exception", m_worker.get().get_fd());
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    shared::SendCallback get_submitter() {
        return [this](buffering::BufferNode &&node) { this->send(std::move(node)); };
    }

    bool resume() {
        core::logger::debug("Sender", "FD `{}` is being executed in resume with stalled = `{}` and closed = `{}`",
                            m_worker.get().get_fd(), m_stalled, m_closed);
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
        core::logger::debug("Sender", "FD `{}` is being marked as closed and so is the Sender too",
                            m_worker.get().get_fd());
        m_closed = true;
    }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }

    [[nodiscard]] bool has_on_error() const noexcept { return !m_on_error; }

  private:
    void arm_write() {
        if (m_closed) {
            core::logger::warning("Sender", "Attempted to write on closed socket `{}`", m_worker.get().get_fd());
            m_stalled = false;
            return;
        }

        auto &view = m_pool.get_view();
        auto [data, size] = view.front();

        if (!data || size == 0) {
            core::logger::info("Sender", "FD `{}` has no data to send", m_worker.get().get_fd());
            m_stalled = false;
            return;
        }

        core::logger::debug("Sender", "FD `{}` attempting to send {} bytes", m_worker.get().get_fd(), size);

        auto [result, status] = m_worker.get().sync_send(data, size);

        switch (status.get_status()) {
        case socket::VALUES::VALID: {
            core::logger::info("Sender", "FD `{}` send {} bytes ", m_worker.get().get_fd(), result);
            view.consume(result);
            m_stalled = false;
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED: {
            core::logger::info("Sender", "FD `{}` send would have blocked, you need to retry when writable",
                               m_worker.get().get_fd());
            m_stalled = false;
            return;
        }
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT:
            core::logger::warning("Sender", "FD `{}` send operation failed with error `{}`", m_worker.get().get_fd(),
                                  result);
            m_closed = true;
            m_on_error(m_worker.get().get_fd(), status.get_value());
            return;
        }
    }

    std::reference_wrapper<Worker> m_worker;
    buffering::BufferPool m_pool;
    shared::ErrorCallback m_on_error;
    bool m_stalled;
    bool m_closed;
};


static_assert(interfaces::io::SyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::sync
