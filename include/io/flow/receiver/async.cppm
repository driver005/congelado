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
    }

    Receiver(Worker &worker, shared::ReadCallback on_read, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)}, m_fatal{false} {
        attach();
    }

    ~Receiver() = default;

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
            const auto fd = m_worker.get().get_fd();
            if (resume()) {
                core::logger::debug("io/recv", "fd {} rescheduled", fd);
                shared::this_handler::shedule();
            } else {
                core::logger::debug("io/recv", "fd {} releasing", fd);
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            detach();
            m_worker.get().async_close();
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            const auto fd = m_worker.get().get_fd();
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("io/recv", "fd {} sys error: {} (code: {})", fd, e.what(), e.code().value());
                m_on_error(fd, e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/recv", "fd {} exception: {}", fd, e.what());
                m_on_error(fd, -1);
            } catch (...) {
                core::logger::warning("io/recv", "fd {} unknown exception", fd);
                m_on_error(fd, -1);
            }
        };
    }

    bool resume() {
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
        m_worker.get().async_read(slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0,
                                  [this, slot](int result) mutable { on_read_complete(slot, result); });
    }

    void on_read_complete(utils::buffering::NodeReader *node, int result) {
        const auto fd = m_worker.get().get_fd();
        if (result <= 0) {
            core::logger::warning("io/recv", "fd {} read error: {}", fd, result);
            m_fatal = true;
            m_on_error(fd, -result);
            return;
        }

        const auto bytes = static_cast<std::size_t>(result);

        core::logger::debug("io/recv", "fd {} rx {} bytes", fd, bytes);

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
