export module io_flow_receiver:async;

import std;
import core_logger;
import shared;
import interfaces;
import io_base_socket;
import utils_buffering;

export namespace io::base::flow::async {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::AsyncReceivable<Worker, Status, std::byte *, Args...>
class Receiver : public shared::HandlerBase {
  public:
    Receiver(Worker &worker) : m_worker{worker}, m_pool{}, m_on_read{nullptr}, m_on_error{nullptr}, m_fatal{false} {
        core::logger::debug("Receiver created for worker with FD `{}`", m_worker.get().get_fd());
    }

    Receiver(Worker &worker, shared::ReadCallback on_read, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)}, m_fatal{false} {
        core::logger::debug("Receiver created for worker with FD `{}`", m_worker.get().get_fd());
        attach();
    }

    ~Receiver() {
        core::logger::debug("Receiver", "destructor called for worker with FD `{}`", m_worker.get().get_fd());
    }

    Receiver(const Receiver &) = delete;
    Receiver &operator=(const Receiver &) = delete;
    Receiver(Receiver &&) = delete;
    Receiver &operator=(Receiver &&) = delete;

    void add_on_read(shared::ReadCallback on_read) { m_on_read = std::move(on_read); }

    void add_on_error(shared::ErrorCallback on_error) { m_on_error = std::move(on_error); }

    void build() {
        if (!m_on_read) {
            throw std::runtime_error("Read callback must be set before building the Receiver");
        }
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Receiver");
        }
        attach();
    }


    std::string_view get_name() const noexcept override { return "Receiver - Async"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            core::logger::debug("Receiver", "FD `{}` on_execute is being executed", m_worker.get().get_fd());
            if (resume()) {
                core::logger::info("Receiver", "FD `{}` rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            } else {
                core::logger::info("Receiver", "FD `{}` to be released", m_worker.get().get_fd());
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            core::logger::debug("Receiver", "FD `{}` on_released is being executed", m_worker.get().get_fd());
            detach();
            m_worker.get().async_close();
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
        core::logger::debug("Receiver", "FD `{}` is being executed in resume with fatal = `{}`",
                            m_worker.get().get_fd(), m_fatal);
        if (m_fatal) {
            return false;
        }

        arm_read();
        return true;
    }

    void attach() { m_worker.get().attach(); }
    void detach() { m_worker.get().detach(); }

    [[nodiscard]] bool get_stalled() const noexcept { return m_fatal; }

  private:
    void arm_read() {
        auto slot = m_pool.acquire();

        core::logger::info("Receiver", "FD `{}` attempting to read up to {} bytes", m_worker.get().get_fd(),
                           slot->get_limit());
        m_worker.get().async_read(slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0,
                                  [this, slot](int result) mutable { on_read_complete(slot, result); });
    }

    void on_read_complete(utils::buffering::NodeReader *node, int result) {
        if (result <= 0) {
            core::logger::warning("Receiver", "FD `{}` read operation failed with error `{}`", m_worker.get().get_fd(),
                                  result);
            m_fatal = true;
            m_on_error(m_worker.get().get_fd(), -result);
            return;
        }

        const auto bytes = static_cast<std::size_t>(result);

        core::logger::info("Receiver", "FD `{}` read {} bytes", m_worker.get().get_fd(), bytes);

        m_pool.notify_read(node, bytes);
        m_on_read(m_pool.get_view());
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ReadCallback m_on_read;
    shared::ErrorCallback m_on_error;
    bool m_fatal;
};


static_assert(
    interfaces::io::AsyncReceivable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus, std::byte *>);

} // namespace io::base::flow::async
